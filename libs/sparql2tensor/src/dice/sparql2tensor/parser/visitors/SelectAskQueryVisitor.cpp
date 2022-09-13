#include "dice/sparql2tensor/parser/visitors/SelectAskQueryVisitor.hpp"
#include "dice/sparql2tensor/expressions/expressions.hpp"

#include <boost/container/flat_set.hpp>

#include <algorithm>
#include <ranges>

namespace dice::sparql2tensor::parser::visitors {

	using namespace dice::sparql2tensor::expressions;

	SelectAskQueryVisitor::SelectAskQueryVisitor(SPARQLQuery *q,
												 triple_store::TripleStore const &ts,
												 robin_hood::unordered_map<std::string, std::string> prefixes,
												 std::chrono::steady_clock::time_point timeout)
		: query(q), triple_store(ts), prefixes_(std::move(prefixes)), timeout_end_time_(timeout) {}

	antlrcpp::Any SelectAskQueryVisitor::visitAskQuery(SparqlParser::AskQueryContext *ctx) {
		query->set_ask();
		if (auto where_clause_ctx = ctx->whereClause(); where_clause_ctx)
			visitWhereClause(where_clause_ctx);
		else
			throw std::runtime_error("Query does not contain a WHERE clause");
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitSelectQuery(SparqlParser::SelectQueryContext *ctx) {
		if (auto where_clause_ctx = ctx->whereClause(); where_clause_ctx)
			visitWhereClause(where_clause_ctx);
		else
			throw std::runtime_error("Query does not contain a WHERE clause");
		if (auto group_clause_ctx = ctx->solutionModifier()->groupClause(); group_clause_ctx)
			visitGroupClause(group_clause_ctx);
		visitSelectClause(ctx->selectClause());
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitSelectClause(SparqlParser::SelectClauseContext *ctx) {
		if (ctx->selectModifier()) {
			if (ctx->selectModifier()->DISTINCT())
				query->set_distinct();
		}
		if (ctx->ASTERISK()) {
			for (auto const &var : vars_in_scope) {
				query->add_projected_variable(var);
				query->track_variable(var);
				query->add_solution_binding(std::make_unique<PrimaryVarExpression>(var, query->tracked_variable_position(var)));
			}
		} else {
			for (auto sel_ctx : ctx->selectVariables()) {
				auto var = visitVar(sel_ctx->var()).as<rdf4cpp::rdf::query::Variable>();
				// the same variable should not be projected multiple times
				if (std::find(query->projected_variables().begin(), query->projected_variables().end(), var) !=
					query->projected_variables().end()) {
					throw std::runtime_error("Variable " + var.backend_handle().variable_backend().n_string() + " is already projected.");
				}
				query->add_projected_variable(var);
				query->register_variable(var);
				// AS expressions should not use variables that are already in scope
				if (sel_ctx->AS()) {
					if (vars_in_scope.contains(var)) {
						throw std::runtime_error("Variable " + var.backend_handle().variable_backend().n_string() + " is already in scope.");
					}
					auto expression = std::move(visitExpression(sel_ctx->expression()).as<std::unique_ptr<SPARQLExpression>>());
					// keep track of the variables appearing in the expression (for checking in case of aggregates)
					if (not dynamic_cast<Aggregate *>(expression.get())) {
						for (auto expr_var : expression->variables()) {
							vars_in_select.insert(expr_var);
						}
					}
					query->add_solution_binding(std::move(expression));
				}
				// the ids of projected variables (not of AS expressions) need to be passed to the query library
				else {
					vars_in_select.insert(var);
					query->track_variable(var);
					query->add_solution_binding(std::make_unique<PrimaryVarExpression>(var, query->tracked_variable_position(var)));
				}
				vars_in_scope.insert(var);
			}
			if (query->projected_variables().empty()) {
				throw std::runtime_error("At least one variable should be projected.");
			}
			// in case of aggregates, check if the variables used in non-aggregate expressions are grouped
			else if (query->contains_aggregates() or not vars_in_group_by.empty()) {
				for (auto var : vars_in_select) {
					if (not vars_in_group_by.contains(var))
						throw std::runtime_error("Variable " + var.backend_handle().variable_backend().n_string() +
												 " is not part of the group key");
				}
			}
		}
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitWhereClause(SparqlParser::WhereClauseContext *ctx) {
		visitGroupGraphPattern(ctx->groupGraphPattern());
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitGroupGraphPattern(SparqlParser::GroupGraphPatternContext *ctx) {
		if (ctx->subSelect())
			throw std::runtime_error("Subqueries are not supported yet");
		else if (auto group_graph_pattern_sub_ctx = ctx->groupGraphPatternSub(); group_graph_pattern_sub_ctx)
			visitGroupGraphPatternSub(group_graph_pattern_sub_ctx);
		else
			throw std::runtime_error("Malformed query");
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitGroupGraphPatternSub(SparqlParser::GroupGraphPatternSubContext *ctx) {
		// push a new entry into the stacks, as we are about to visit a graph pattern
		group_patterns.emplace_back();
		triples_blocks.emplace_back();
		optional_blocks.emplace_back();
		filter_blocks.emplace_back();
		visitWellDesignedPattern(ctx, {});
		// pop the top entry of the stacks, as we have finished visiting the graph pattern
		filter_blocks.pop_back();
		optional_blocks.pop_back();
		triples_blocks.pop_back();
		group_patterns.pop_back();
		return nullptr;
	}

	void SelectAskQueryVisitor::visitWellDesignedPattern(SparqlParser::GroupGraphPatternSubContext *ctx,
														 std::vector<SparqlParser::GroupOrUnionGraphPatternContext *> gou_ctxs) {
		// store the context of the first triples block, if it is provided
		if (auto triples_block = ctx->triplesBlock(); triples_block)
			triples_blocks.back().push_back(triples_block);
		// iterate over all GroupGraphPatternSubs
		for (auto sub_ctx : ctx->groupGraphPatternSubList()) {
			if (sub_ctx->graphPatternNotTriples()->bind() or sub_ctx->graphPatternNotTriples()->inlineData() or
				sub_ctx->graphPatternNotTriples()->minusGraphPattern() or sub_ctx->graphPatternNotTriples()->serviceGraphPattern())
				throw std::runtime_error("Feature not supported: " + sub_ctx->graphPatternNotTriples()->getText());
			if (auto graph_pattern_not_triples_ctx = sub_ctx->graphPatternNotTriples(); graph_pattern_not_triples_ctx) {
				// store all GroupOrUnionGraphPatterns that appear in the pattern
				if (auto group_or_union_graph_pattern_ctx = graph_pattern_not_triples_ctx->groupOrUnionGraphPattern(); group_or_union_graph_pattern_ctx)
					gou_ctxs.push_back(group_or_union_graph_pattern_ctx);
				// store all OptionalGraphPatterns that appear in the pattern
				else if (auto optional_graph_pattern_ctx = sub_ctx->graphPatternNotTriples()->optionalGraphPattern(); optional_graph_pattern_ctx)
					optional_blocks.back().push_back(optional_graph_pattern_ctx);
				// store all FilterPatterns that appear in the pattern
				else if (auto filter_ctx = sub_ctx->graphPatternNotTriples()->filter(); filter_ctx)
					filter_blocks.back().push_back(filter_ctx);
			}
			// store all triples blocks that appear in the pattern
			if (auto triples_block_ctx = sub_ctx->triplesBlock(); triples_block_ctx)
				triples_blocks.back().push_back(triples_block_ctx);
		}
		// the current pattern does not contain any GroupOrUnionGraphPatterns
		if (gou_ctxs.empty()) {
			// visit all triples blocks first
			for (auto tb_ctx : triples_blocks.back()) {
				visitTriplesBlock(tb_ctx);
			}
			// visit all filters
			for (auto f_ctx : filter_blocks.back()) {
				visitFilter(f_ctx);
			}
			// if we are in an optional pattern we need to capture dependencies
			if (not opt_operands.empty()) {
				// dependencies with parent group
				group_dependencies(group_patterns[group_patterns.size() - 2], group_patterns.back());
				// cartesian connections between optional patterns
				for (auto cur_op : group_patterns.back()) {
					for (auto opt_op : opt_operands.back()) {
						// do not connect groups of the same union pattern
						if (std::ranges::find(union_operands.back(), opt_op) == union_operands.back().end()) {
							query->add_connection(cur_op, opt_op);
						}
					}
				}
				for (auto cur_op : group_patterns.back()) {
					union_operands.back().push_back(cur_op);
					opt_operands.back().push_back(cur_op);
				}
			}
			opt_operands.emplace_back();
			union_operands.emplace_back();
			// visit all optional patterns
			for (auto opt_ctx : optional_blocks.back()) {
				// push a new vector into the stacks, as we are going to visit a new graph pattern
				group_patterns.emplace_back();
				triples_blocks.emplace_back();
				optional_blocks.emplace_back();
				filter_blocks.emplace_back();
				visitWellDesignedPattern(opt_ctx->groupGraphPattern()->groupGraphPatternSub(), {});
				union_operands.back().clear();
				// clear the vector from the operands of the visited graph pattern
				// the top vector of the stack is shared across all optional subgraph pattern of the current graph pattern
				union_operands.back().clear();
				// pop the top vector from the stack, as we have finished processing the graph pattern
				filter_blocks.pop_back();
				optional_blocks.pop_back();
				triples_blocks.pop_back();
				group_patterns.pop_back();
			}
			union_operands.pop_back();
			opt_operands.pop_back();
			// prepare for the next union
			group_patterns.back().clear();
		}
		// the pattern contains at least one GroupOrUnionGraphPattern
		// in case of multiple GroupOrUnionGraphPatterns, join operations are distributed over unions
		else {
			SparqlParser::GroupOrUnionGraphPatternContext *cur_gou_ctx = gou_ctxs.back();
			gou_ctxs.pop_back();
			size_t current_tbs = triples_blocks.back().size();
			size_t current_opts = optional_blocks.back().size();
			size_t current_filters = filter_blocks.back().size();
			// visit each group graph pattern of the GroupOrUnionGraphPattern
			// while visiting each group graph pattern, the triples and optional blocks stored until this point will also be visited
			for (auto grp_ctx : cur_gou_ctx->groupGraphPattern()) {
				visitWellDesignedPattern(grp_ctx->groupGraphPatternSub(), gou_ctxs);
				// we resize the vectors in order to keep only the blocks that were present before visiting grp_ctx
				triples_blocks.back().resize(current_tbs);
				optional_blocks.back().resize(current_opts);
				filter_blocks.back().resize(current_filters);
			}
		}
	}

	antlrcpp::Any SelectAskQueryVisitor::visitFilter(SparqlParser::FilterContext *ctx) {
		std::unique_ptr<SPARQLExpression> expression;
		if (auto expr_ctx = ctx->constraint()->expression(); expr_ctx)
			expression = std::move(visitExpression(expr_ctx).as<std::unique_ptr<SPARQLExpression>>());
		else if (auto built_in_call_ctx = ctx->constraint()->builtInCall(); built_in_call_ctx)
			expression = std::move(visitBuiltInCall(built_in_call_ctx).as<std::unique_ptr<SPARQLExpression>>());
		else
			throw std::runtime_error("function calls are not supported");
		if (auto and_ctx = dynamic_cast<LogicalAndExpression *>(expression.get()); and_ctx == nullptr) {
			auto operand_desc = query->add_filter_expr(std::move(expression), triple_store);
			for (auto desc : group_patterns.back()) {
				query->add_dependency(operand_desc, desc);
			}
			group_patterns.back().push_back(operand_desc);
		} else {
			// in case of ConditionalAndExpressions, create a unique vertex for each operand
			// this allows for returning false as soon as an operand evaluates to false
			for (auto &expr : and_ctx->expressions()) {
				auto operand_desc = query->add_filter_expr(std::move(expr), triple_store);
				for (auto desc : group_patterns.back()) {
					query->add_dependency(operand_desc, desc);
				}
				group_patterns.back().push_back(operand_desc);
			}
		}
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitTriplesBlock(SparqlParser::TriplesBlockContext *ctx) {
		for (auto sub_ctx : ctx->triplesSameSubjectPath())
			visitTriplesSameSubjectPath(sub_ctx);
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitTriplesSameSubjectPath(SparqlParser::TriplesSameSubjectPathContext *ctx) {
		if (ctx->varOrTerm() and ctx->propertyListPathNotEmpty()) {
			active_subject = visitVarOrTerm(ctx->varOrTerm());
			if (active_subject.is_variable()) {
				auto var = rdf4cpp::rdf::query::Variable(active_subject);
				query->register_variable(var);
				vars_in_scope.insert(var);
			}
			visitPropertyListPathNotEmpty(ctx->propertyListPathNotEmpty());
		} else if (ctx->triplesNodePath() and ctx->propertyListPath()) {
			return nullptr;
		}
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitPropertyListPathNotEmpty(SparqlParser::PropertyListPathNotEmptyContext *ctx) {
		if (ctx->verbPath()) {
			active_predicate = visitPath(ctx->verbPath()->path());
		} else {
			auto var = visitVar(ctx->verbSimple()->var()).as<rdf4cpp::rdf::query::Variable>();
			query->register_variable(var);
			vars_in_scope.insert(var);
			active_predicate = rdf4cpp::rdf::Node(var);
		}
		if (not ctx->objectListPath())
			throw std::runtime_error("Triple requires at least one object");
		visitObjectListPath(ctx->objectListPath());
		for (auto prop_ctx : ctx->propertyListPathNotEmptyList()) {
			if (prop_ctx->verbPath()) {
				active_predicate = visitPath(prop_ctx->verbPath()->path()).as<rdf4cpp::rdf::Node>();
			} else {
				auto var = visitVar(prop_ctx->verbSimple()->var()).as<rdf4cpp::rdf::query::Variable>();
				query->register_variable(var);
				vars_in_scope.insert(var);
				active_predicate = rdf4cpp::rdf::Node(var);
			}
			if (not prop_ctx->objectList())
				throw std::runtime_error("Triple requires at least one object");
			visitObjectList(prop_ctx->objectList());
		}
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitVarOrTerm(SparqlParser::VarOrTermContext *ctx) {
		if (ctx->var()) {
			return rdf4cpp::rdf::Node(visitVar(ctx->var()).as<rdf4cpp::rdf::query::Variable>());
		} else {
			if (auto iri_ctx = ctx->graphTerm()->iri())
				return rdf4cpp::rdf::Node(visitIri(iri_ctx).as<rdf4cpp::rdf::IRI>());
			else if (auto blank_node_ctx = ctx->graphTerm()->blankNode(); blank_node_ctx)
				return rdf4cpp::rdf::Node(visitBlankNode(blank_node_ctx).as<rdf4cpp::rdf::query::Variable>());
			else if (auto rdf_literal_ctx = ctx->graphTerm()->rdfLiteral(); rdf_literal_ctx)
				return rdf4cpp::rdf::Node(visitRdfLiteral(rdf_literal_ctx).as<rdf4cpp::rdf::Literal>());
			else if (auto boolean_literal_ctx = ctx->graphTerm()->booleanLiteral(); boolean_literal_ctx)
				return rdf4cpp::rdf::Node(visitBooleanLiteral(boolean_literal_ctx).as<rdf4cpp::rdf::Literal>());
			else if (auto numberic_literal_ctx = ctx->graphTerm()->numericLiteral(); numberic_literal_ctx)
				return rdf4cpp::rdf::Node(visitNumericLiteral(numberic_literal_ctx).as<rdf4cpp::rdf::Literal>());
			else
				throw std::runtime_error("RDF collections are not supported yet.");
		}
	}

	antlrcpp::Any SelectAskQueryVisitor::visitIri(SparqlParser::IriContext *ctx) {
		if (ctx->IRIREF()) {
			auto iri = ctx->IRIREF()->getText();
			return rdf4cpp::rdf::IRI(iri.substr(1, iri.size() - 2));
		}
		std::string predicate = ctx->prefixedName()->PNAME_LN()->getText();
		std::size_t split = predicate.find(':');
		try {
			return rdf4cpp::rdf::IRI(prefixes_.at(predicate.substr(0, split)) + predicate.substr(split + 1));
		} catch (...) {
			throw std::out_of_range("Prefix " + predicate.substr(0, split) + " not declared.");
		}
	}

	antlrcpp::Any SelectAskQueryVisitor::visitBlankNode(SparqlParser::BlankNodeContext *ctx) {
		if (auto blank_node_label_ctx = ctx->BLANK_NODE_LABEL(); blank_node_label_ctx)
			return rdf4cpp::rdf::query::Variable(blank_node_label_ctx->getText().substr(2), true);
		else
			throw std::runtime_error("BlankNode ANON not supported.");
	}

	antlrcpp::Any SelectAskQueryVisitor::visitVar(SparqlParser::VarContext *ctx) {
		return rdf4cpp::rdf::query::Variable(ctx->getText().substr(1));
	}

	antlrcpp::Any SelectAskQueryVisitor::visitObjectListPath(SparqlParser::ObjectListPathContext *ctx) {
		for (auto objp_ctx : ctx->objectPath())
			visitObjectPath(objp_ctx);
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitObjectList(SparqlParser::ObjectListContext *ctx) {
		for (auto obj_ctx : ctx->object())
			visitObject(obj_ctx);
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitObjectPath(SparqlParser::ObjectPathContext *ctx) {
		if (auto var_or_term_ctx = ctx->graphNodePath()->varOrTerm(); var_or_term_ctx) {
			rdf4cpp::rdf::Node obj = visitVarOrTerm(var_or_term_ctx);
			if (obj.is_variable()) {
				auto var = rdf4cpp::rdf::query::Variable(obj);
				query->register_variable(var);
				vars_in_scope.insert(var);
			}
			rdf4cpp::rdf::query::TriplePattern triple_pattern{active_subject, active_predicate, obj};
			auto operand_desc = query->add_triple_pattern(triple_pattern, triple_store);
			// bgp dependencies
			for (auto desc : group_patterns.back()) {
				query->add_dependency(operand_desc, desc);
			}
			group_patterns.back().push_back(operand_desc);
		} else {
			throw std::runtime_error("not supported");
		}
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitObject(SparqlParser::ObjectContext *ctx) {
		if (auto var_or_term_ctx = ctx->graphNode()->varOrTerm(); var_or_term_ctx) {
			rdf4cpp::rdf::Node obj = visitVarOrTerm(var_or_term_ctx);
			if (obj.is_variable()) {
				auto var = rdf4cpp::rdf::query::Variable(obj);
				query->register_variable(var);
				vars_in_scope.insert(var);
			}
			rdf4cpp::rdf::query::TriplePattern triple_pattern{active_subject, active_predicate, obj};
			auto operand_desc = query->add_triple_pattern(triple_pattern, triple_store);
			// bgp dependencies
			for (auto desc : group_patterns.back()) {
				query->add_dependency(operand_desc, desc);
			}
			group_patterns.back().push_back(operand_desc);
		} else {
			throw std::runtime_error("not supported");
		}
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitPath(SparqlParser::PathContext *ctx) {
		if (auto path_alternative_ctx = ctx->pathAlternative(); path_alternative_ctx)
			return visitPathAlternative(path_alternative_ctx);
		else
			throw std::runtime_error("Malformed query.");
	}

	antlrcpp::Any SelectAskQueryVisitor::visitPathAlternative(SparqlParser::PathAlternativeContext *ctx) {
		if (ctx->pathSequence().size() > 1)
			throw std::runtime_error("Property paths are not supported yet");
		return visitPathSequence(ctx->pathSequence(0));
	}

	antlrcpp::Any SelectAskQueryVisitor::visitPathSequence(SparqlParser::PathSequenceContext *ctx) {
		if (ctx->pathEltOrInverse().size() > 1)
			throw std::runtime_error("Property paths are not supported yet");
		return visitPathEltOrInverse(ctx->pathEltOrInverse(0));
	}

	antlrcpp::Any SelectAskQueryVisitor::visitPathEltOrInverse(SparqlParser::PathEltOrInverseContext *ctx) {
		if (ctx->INVERSE())
			throw std::runtime_error("Property paths are not supported yet");
		return visitPathElt(ctx->pathElt());
	}

	antlrcpp::Any SelectAskQueryVisitor::visitPathElt(SparqlParser::PathEltContext *ctx) {
		auto path_primary_ctx = ctx->pathPrimary();
		if (auto iri_ctx = path_primary_ctx->iri(); iri_ctx)
			return rdf4cpp::rdf::Node(visitIri(iri_ctx).as<rdf4cpp::rdf::IRI>());
		else if (path_primary_ctx->A())
			return rdf4cpp::rdf::Node(rdf4cpp::rdf::IRI("http://www.w3.org/1999/02/22-rdf-syntax-ns#type"));
		else if (path_primary_ctx->NEGATION())
			throw std::runtime_error("Property paths are not supported yet");
		else
			return visitPath(ctx->pathPrimary()->path());
	}

	/* solution modifiers */

	antlrcpp::Any SelectAskQueryVisitor::visitGroupClause(SparqlParser::GroupClauseContext *ctx) {
		for (auto group_condition : ctx->groupCondition()) {
			if (group_condition->builtInCall()) {
				return nullptr;//built in call visitor
			} else if (group_condition->functionCall()) {
				return nullptr;//function call visitor
			} else if (group_condition->var()) {
				auto var = visitVar(group_condition->var()).as<rdf4cpp::rdf::query::Variable>();
				query->track_variable(var);
				query->add_grouping_expression(std::make_unique<PrimaryVarExpression>(var, query->tracked_variable_position(var)));
				vars_in_group_by.insert(var);
			} else if (group_condition->AS()) {
				return nullptr;// need to visit expression and track/assign alias
			} else {
				throw std::runtime_error("Unsupported GroupCondition");
			}
		}
		return nullptr;
	}

	/* expressions */

	antlrcpp::Any SelectAskQueryVisitor::visitExpression(SparqlParser::ExpressionContext *ctx) {
		std::unique_ptr<SPARQLExpression> expr;
		if (auto base_ctx = dynamic_cast<SparqlParser::BaseExpressionContext *>(ctx); base_ctx) {
			expr = std::move(visitPrimaryExpression(base_ctx->primaryExpression()).as<std::unique_ptr<SPARQLExpression>>());
		} else if (auto and_ctx = dynamic_cast<SparqlParser::ConditionalAndExpressionContext *>(ctx); and_ctx) {
			expr = std::move(visitConditionalAndExpression(and_ctx).as<std::unique_ptr<SPARQLExpression>>());
		} else if (auto or_ctx = dynamic_cast<SparqlParser::ConditionalOrExpressionContext *>(ctx); or_ctx) {
			expr = std::move(visitConditionalOrExpression(or_ctx).as<std::unique_ptr<SPARQLExpression>>());
		} else if (auto relational_ctx = dynamic_cast<SparqlParser::RelationalExpressionContext *>(ctx); relational_ctx) {
			expr = std::move(visitRelationalExpression(relational_ctx).as<std::unique_ptr<SPARQLExpression>>());
		} else {
			throw std::runtime_error("Unsupported Expression: " + ctx->getText());
		}
		return expr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitConditionalAndExpression(SparqlParser::ConditionalAndExpressionContext *ctx) {
		std::unique_ptr<SPARQLExpression> logical_and_expr;
		std::vector<std::unique_ptr<SPARQLExpression>> expressions;
		for (auto expr_ctx : ctx->expression()) {
			expressions.push_back(std::move(visitExpression(expr_ctx).as<std::unique_ptr<SPARQLExpression>>()));
		}
		logical_and_expr = std::make_unique<LogicalAndExpression>(std::move(expressions));
		return logical_and_expr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitConditionalOrExpression(SparqlParser::ConditionalOrExpressionContext *ctx) {
		std::unique_ptr<SPARQLExpression> logical_or_expr;
		std::vector<std::unique_ptr<SPARQLExpression>> expressions;
		for (auto expr_ctx : ctx->expression()) {
			expressions.push_back(std::move(visitExpression(expr_ctx).as<std::unique_ptr<SPARQLExpression>>()));
		}
		logical_or_expr = std::make_unique<LogicalOrExpression>(std::move(expressions));
		return logical_or_expr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitRelationalExpression(SparqlParser::RelationalExpressionContext *ctx) {
		std::unique_ptr<SPARQLExpression> expression;
		std::unique_ptr<SPARQLExpression> lhs_op = std::move(visitExpression(ctx->expression(0)).as<std::unique_ptr<SPARQLExpression>>());
		std::unique_ptr<SPARQLExpression> rhs_op = std::move(visitExpression(ctx->expression(1)).as<std::unique_ptr<SPARQLExpression>>());
		if (ctx->EQUAL())
			expression = std::make_unique<EqualsExpression>(std::move(lhs_op), std::move(rhs_op));
		else if (ctx->NOT_EQUAL())
			expression = std::make_unique<NotEqualsExpression>(std::move(lhs_op), std::move(rhs_op));
		else if (ctx->GREATER())
			expression = std::make_unique<GreaterExpression>(std::move(lhs_op), std::move(rhs_op));
		else if (ctx->GREATER_EQUAL())
			expression = std::make_unique<GreaterEqualsExpression>(std::move(lhs_op), std::move(rhs_op));
		else if (ctx->LESS())
			expression = std::make_unique<LessExpression>(std::move(lhs_op), std::move(rhs_op));
		else if (ctx->LESS_EQUAL())
			expression = std::make_unique<LessEqualsExpression>(std::move(lhs_op), std::move(rhs_op));
		else
			throw std::runtime_error("Expression not supported: " + ctx->getText());
		return expression;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitPrimaryExpression(SparqlParser::PrimaryExpressionContext *ctx) {
		std::unique_ptr<SPARQLExpression> expr;
		if (ctx->var()) {
			auto var = visitVar(ctx->var()).as<rdf4cpp::rdf::query::Variable>();
			query->register_variable(var);
			query->track_variable(var);
			vars_in_scope.insert(var);// todo: this needs to be changed when minus is introduced
			expr = std::make_unique<PrimaryVarExpression>(var, query->tracked_variable_position(var));
		} else if (ctx->rdfLiteral()) {
			auto rdf_literal = visitRdfLiteral(ctx->rdfLiteral()).as<rdf4cpp::rdf::Literal>();
			expr = std::make_unique<PrimaryLiteralExpression>(rdf_literal);
		} else if (ctx->booleanLiteral()) {
			auto boolean_literal = visitBooleanLiteral(ctx->booleanLiteral()).as<rdf4cpp::rdf::Literal>();
			expr = std::make_unique<PrimaryLiteralExpression>(boolean_literal);
		} else if (ctx->numericLiteral()) {
			auto numeric_literal = visitNumericLiteral(ctx->numericLiteral()).as<rdf4cpp::rdf::Literal>();
			expr = std::make_unique<PrimaryLiteralExpression>(numeric_literal);
		} else if (auto built_in_call_ctx = ctx->builtInCall(); built_in_call_ctx) {
			if (auto aggregate_ctx = built_in_call_ctx->aggregate(); aggregate_ctx) {
				expr = std::move(visitAggregate(aggregate_ctx).as<std::unique_ptr<SPARQLExpression>>());
			} else {
				expr = std::move(visitBuiltInCall(built_in_call_ctx).as<std::unique_ptr<SPARQLExpression>>());
			}
		} else if (auto iri_or_function_ctx = ctx->iriRefOrFunction(); iri_or_function_ctx) {
			if (iri_or_function_ctx->argList()) {
				throw std::runtime_error("Functions are currently not supported");
			} else {
				auto iri = visitIri(iri_or_function_ctx->iri()).as<rdf4cpp::rdf::IRI>();
				expr = std::make_unique<PrimaryIRIExpression>(iri);
			}
		} else {
			expr = std::move(visitExpression(ctx->expression()).as<std::unique_ptr<SPARQLExpression>>());
		}
		return expr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitBuiltInCall(SparqlParser::BuiltInCallContext *ctx) {
		// treat EXISTS differently from the other built-in calls
		if (auto exists_ctx = ctx->existsFunction(); exists_ctx) {
			return visitExists(exists_ctx->groupGraphPattern(), false);
		}
		if (auto not_exists_ctx = ctx->notExistsFunction(); not_exists_ctx) {
			return visitExists(not_exists_ctx->groupGraphPattern(), true);
		}
		// built-in function calls
		std::unique_ptr<SPARQLExpression> expr;
		if (ctx->ISIRI() or ctx->ISURI()) {
			expr = std::make_unique<IsIRI>(std::move(visitExpression(ctx->expression(0)).as<std::unique_ptr<SPARQLExpression>>()));
		} else if (ctx->ISBLANK()) {
			expr = std::make_unique<IsBlank>(std::move(visitExpression(ctx->expression(0)).as<std::unique_ptr<SPARQLExpression>>()));
		} else if (ctx->ISLITERAL()) {
			expr = std::make_unique<IsLiteral>(std::move(visitExpression(ctx->expression(0)).as<std::unique_ptr<SPARQLExpression>>()));
		} else if (ctx->DATATYPE()) {
			expr = std::make_unique<Datatype>(std::move(visitExpression(ctx->expression(0)).as<std::unique_ptr<SPARQLExpression>>()));
		} else if (ctx->STR()) {
			expr = std::make_unique<Str>(std::move(visitExpression(ctx->expression(0)).as<std::unique_ptr<SPARQLExpression>>()));
		} else if (ctx->CONTAINS()) {
			expr = std::make_unique<Contains>(std::move(visitExpression(ctx->expression(0)).as<std::unique_ptr<SPARQLExpression>>()),
											  std::move(visitExpression(ctx->expression(1)).as<std::unique_ptr<SPARQLExpression>>()));
		} else if (ctx->LANG()) {
			expr = std::make_unique<Lang>(std::move(visitExpression(ctx->expression(0)).as<std::unique_ptr<SPARQLExpression>>()));
		} else if (ctx->STRSTARTS()) {
			expr = std::make_unique<StrStarts>(std::move(visitExpression(ctx->expression(0)).as<std::unique_ptr<SPARQLExpression>>()),
											   std::move(visitExpression(ctx->expression(1)).as<std::unique_ptr<SPARQLExpression>>()));
		} else if (ctx->STRENDS()) {
			expr = std::make_unique<StrEnds>(std::move(visitExpression(ctx->expression(0)).as<std::unique_ptr<SPARQLExpression>>()),
											 std::move(visitExpression(ctx->expression(1)).as<std::unique_ptr<SPARQLExpression>>()));
		} else if (ctx->STRLANG()) {
			expr = std::make_unique<StrLang>(std::move(visitExpression(ctx->expression(0)).as<std::unique_ptr<SPARQLExpression>>()),
											 std::move(visitExpression(ctx->expression(1)).as<std::unique_ptr<SPARQLExpression>>()));
		} else if (ctx->LANGMATCHES()) {
			expr = std::make_unique<LangMatches>(std::move(visitExpression(ctx->expression(0)).as<std::unique_ptr<SPARQLExpression>>()),
												 std::move(visitExpression(ctx->expression(1)).as<std::unique_ptr<SPARQLExpression>>()));
		} else {
			throw std::runtime_error("Unsupported built-in function: " + ctx->getText());
		}
		return expr;
	}

	std::unique_ptr<expressions::SPARQLExpression> SelectAskQueryVisitor::visitExists(SparqlParser::GroupGraphPatternContext *ctx, bool is_not) {
		// treat the pattern of the EXIST function as a subquery
		SPARQLQuery sub_query;
		SelectAskQueryVisitor sub_query_visitor(&sub_query, triple_store, prefixes_, timeout_end_time_);
		sub_query_visitor.visitGroupGraphPattern(ctx);
		// associate the variables of the EXIST subquery with the current query
		boost::container::flat_map<char, size_t> subquery_var_ids_positions{};
		std::vector<rdf4cpp::rdf::query::Variable> vars{};
		for (auto const &sub_query_var : sub_query_visitor.vars_in_scope) {
			if (vars_in_scope.contains(sub_query_var)) {
				vars.push_back(sub_query_var);
				query->track_variable(sub_query_var);
				subquery_var_ids_positions[sub_query.variable_id(sub_query_var)] = query->tracked_variable_position(sub_query_var);
			}
		}
		return std::make_unique<Exists>(std::move(vars), std::move(subquery_var_ids_positions),
										sub_query.raw_query(), is_not, timeout_end_time_);
	}

	antlrcpp::Any SelectAskQueryVisitor::visitAggregate(SparqlParser::AggregateContext *ctx) {
		query->set_aggregates();
		std::unique_ptr<SPARQLExpression> expr;
		std::unique_ptr<SPARQLExpression> nested_expr;
		if (ctx->expression()) {
			nested_expr = std::move(visitExpression(ctx->expression()).as<std::unique_ptr<SPARQLExpression>>());
			if (dynamic_cast<Aggregate *>(nested_expr.get())) {
				throw std::runtime_error("Nested aggregates are not allowed");
			}
		}
		if (ctx->DISTINCT()) {
			if (ctx->COUNT()) {
				if (ctx->ASTERISK()) {
					for (auto const &var : vars_in_scope) {
						query->track_variable(var);
					}
					expr = std::make_unique<CountStarDistinct>();
				} else {
					expr = std::make_unique<CountDistinct>(std::move(nested_expr));
				}
			} else if (ctx->MIN()) {
				expr = std::make_unique<Min>(std::move(nested_expr));
			} else if (ctx->MAX()) {
				expr = std::make_unique<Max>(std::move(nested_expr));
			} else if (ctx->SAMPLE()) {
				expr = std::make_unique<Sample>(std::move(nested_expr));
			} else {
				throw std::runtime_error("not supported");
			}
		} else {
			if (ctx->COUNT()) {
				if (ctx->ASTERISK()) {
					for (auto const &var : vars_in_scope) {
						query->track_variable(var);
					}
					expr = std::make_unique<CountStar>();
				} else {
					expr = std::make_unique<Count>(std::move(nested_expr));
				}
			} else if (ctx->MIN()) {
				expr = std::make_unique<Min>(std::move(nested_expr));
			} else if (ctx->MAX()) {
				expr = std::make_unique<Max>(std::move(nested_expr));
			} else if (ctx->SAMPLE()) {
				expr = std::make_unique<Sample>(std::move(nested_expr));
			} else {
				throw std::runtime_error("not supported");
			}
		}
		return expr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitRdfLiteral(SparqlParser::RdfLiteralContext *ctx) {
		std::string value = visitString(ctx->string());
		if (auto iri_ctx = ctx->iri(); iri_ctx)
			return rdf4cpp::rdf::Literal(value, visitIri(iri_ctx).as<rdf4cpp::rdf::IRI>());
		else if (auto langtag_ctx = ctx->LANGTAG(); langtag_ctx)
			return rdf4cpp::rdf::Literal(value, langtag_ctx->getText().substr(1));
		else
			return rdf4cpp::rdf::Literal(value);
	}

	antlrcpp::Any SelectAskQueryVisitor::visitNumericLiteral(SparqlParser::NumericLiteralContext *ctx) {
		auto number = ctx->getText();
		if (auto pos_literal_ctx = ctx->numericLiteralPositive(); pos_literal_ctx) {
			if (pos_literal_ctx->DECIMAL_POSITIVE())
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#decimal"));
			else if (pos_literal_ctx->DOUBLE_POSITIVE())
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#double"));
			else
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"));
		} else if (auto neg_literal_ctx = ctx->numericLiteralNegative(); neg_literal_ctx) {
			if (neg_literal_ctx->DECIMAL_NEGATIVE())
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#decimal"));
			else if (neg_literal_ctx->DOUBLE_NEGATIVE())
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#double"));
			else
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"));
		} else {
			auto unsigned_literal_ctx = ctx->numericLiteralUnsigned();
			if (unsigned_literal_ctx->DECIMAL())
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#decimal"));
			else if (unsigned_literal_ctx->DOUBLE())
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#double"));
			else
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"));
		}
	}

	antlrcpp::Any SelectAskQueryVisitor::visitBooleanLiteral(SparqlParser::BooleanLiteralContext *ctx) {
		if (ctx->TRUE())
			return rdf4cpp::rdf::Literal("true", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean"));
		else
			return rdf4cpp::rdf::Literal("false", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean"));
	}

	antlrcpp::Any SelectAskQueryVisitor::visitString(SparqlParser::StringContext *ctx) {
		std::string value = ctx->getText();
		if (ctx->STRING_LITERAL1() or ctx->STRING_LITERAL2())
			return value.substr(1, value.size() - 2);
		else
			return value.substr(3, value.size() - 6);
	}

	void SelectAskQueryVisitor::group_dependencies(std::vector<uint8_t> const &prev_group,
												   std::vector<uint8_t> const &cur_group,
												   bool bidirectional) {
		// iterate over the operands (nodes) of the previous group
		for (const auto &prev_tp : prev_group) {
			// iterate over the operands (nodes) of the current group
			for (const auto &cur_tp : cur_group) {
				query->add_dependency(prev_tp, cur_tp, bidirectional);
			}
		}
	}

}// namespace dice::sparql2tensor::parser::visitors