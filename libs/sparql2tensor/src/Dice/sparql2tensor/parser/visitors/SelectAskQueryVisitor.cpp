#include "Dice/sparql2tensor/parser/visitors/SelectAskQueryVisitor.hpp"
#include "Dice/sparql2tensor/expressions/expressions.hpp"

namespace Dice::sparql2tensor::parser::visitors {

	using namespace Dice::sparql2tensor::expressions;

	antlrcpp::Any SelectAskQueryVisitor::visitAskQuery(SparqlParser::AskQueryContext *ctx) {
		if (ctx->whereClause())
			visitWhereClause(ctx->whereClause());
		else
			throw std::runtime_error("Query does not contain a WHERE clause");
		query->ask_ = true;
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitSelectQuery(SparqlParser::SelectQueryContext *ctx) {
		if (ctx->whereClause())
			visitWhereClause(ctx->whereClause());
		else
			throw std::runtime_error("Query does not contain a WHERE clause");
		if (ctx->solutionModifier()->groupClause())
			visitGroupClause(ctx->solutionModifier()->groupClause());
		visitSelectClause(ctx->selectClause());
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitSelectClause(SparqlParser::SelectClauseContext *ctx) {
		std::vector<std::unique_ptr<Expression>> select_expressions;
		if (ctx->selectModifier()) {
			if (ctx->selectModifier()->DISTINCT())
				query->distinct_ = true;
		}
		if (ctx->ASTERISK()) {
			for (auto const& var : vars_in_scope) {
				query->projected_variables_.push_back(var);
				select_expressions.push_back(std::make_unique<PrimaryVarExpression>(var, query->tracked_variables_[var]));
			}
		} else {
			for (auto sel_ctx : ctx->selectVariables()) {
				auto var = visitVar(sel_ctx->var()).as<rdf4cpp::rdf::query::Variable>();
				// the same variable should not be projected multiple times
				if (std::find(query->projected_variables_.begin(), query->projected_variables_.end(), var) !=
					query->projected_variables_.end()) {
					throw std::runtime_error("Variable " + var.backend_handle().variable_backend().n_string() + " is already projected." );
				}
				query->projected_variables_.push_back(var);
				// AS expressions should not use variables that are already in scope
				if (sel_ctx->AS()) {
					if (vars_in_scope.contains(var)) {
						throw std::runtime_error("Variable " + var.backend_handle().variable_backend().n_string() + " is already in scope." );
					}
					select_expressions.push_back(std::move(visitExpression(sel_ctx->expression()).as<std::unique_ptr<Expression>>()));
					// in case of aggregates, check if non group key variables are projected
				}
				// the ids of projected variables (not of AS expressions) need to be passed to the query library
				else {
					track_variable(var);
					select_expressions.push_back(std::make_unique<PrimaryVarExpression>(var, query->tracked_variables_[var]));
				}
				vars_in_scope.insert(var);
			}
			if (query->projected_variables_.empty()) {
				throw std::runtime_error("At least one variable should be projected.");
			} else if (query->contains_aggregates_) {
				// check if there are non-aggregated and non group key variables in the select clause
				for (auto const &select_expr : select_expressions) {
					if (dynamic_cast<Aggregate *>(select_expr.get()))
						continue;
					auto expr_vars = select_expr->variables();
					for (auto var : expr_vars) {
						if (std::find(vars_in_group_by.begin(), vars_in_group_by.end(), var) == vars_in_group_by.end())
							throw std::runtime_error("Variable " + var.backend_handle().variable_backend().n_string()
													 + " is not part of the group key");
					}
				}
			}
		}
		query->solution_ = ExpressionList(std::move(select_expressions));
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitWhereClause(SparqlParser::WhereClauseContext *ctx) {
		group_patterns.emplace_back();
		triples_blocks.emplace_back();
		optional_blocks.emplace_back();
		visitGroupGraphPattern(ctx->groupGraphPattern());
		optional_blocks.pop_back();
		triples_blocks.pop_back();
		group_patterns.pop_back();
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitGroupGraphPattern(SparqlParser::GroupGraphPatternContext *ctx) {
		if (ctx->subSelect())
			throw std::runtime_error("Subqueries are not supported yet");
		else if (ctx->groupGraphPatternSub())
			visitGroupGraphPatternSub(ctx->groupGraphPatternSub());
		else
			throw std::runtime_error("Malformed query");
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitGroupGraphPatternSub(SparqlParser::GroupGraphPatternSubContext *ctx) {
		visitWellDesignedPattern(ctx, {});
		return nullptr;
	}

	void SelectAskQueryVisitor::visitWellDesignedPattern(SparqlParser::GroupGraphPatternSubContext *ctx,
														 std::vector<SparqlParser::GroupOrUnionGraphPatternContext *> gou_ctxs) {
		/*
		 * The visitor first collects all triple patterns, group/union graph patterns and optional patterns
		 * For each union all triple patterns and optional patterns that are explored so far are visited
		 */
		if (ctx->triplesBlock())
			triples_blocks.back().push_back(ctx->triplesBlock());
		for (auto sub_ctx : ctx->groupGraphPatternSubList()) {
			if (sub_ctx->graphPatternNotTriples()) {
				if (sub_ctx->graphPatternNotTriples()->groupOrUnionGraphPattern())
					gou_ctxs.push_back(sub_ctx->graphPatternNotTriples()->groupOrUnionGraphPattern());
				else if (sub_ctx->graphPatternNotTriples()->optionalGraphPattern())
					optional_blocks.back().push_back(sub_ctx->graphPatternNotTriples()->optionalGraphPattern());
			}
			if (sub_ctx->triplesBlock())
				triples_blocks.back().push_back(sub_ctx->triplesBlock());
		}
		if (gou_ctxs.empty()) {
			for (auto tb_ctx : triples_blocks.back()) {
				visitTriplesBlock(tb_ctx);
			}
			// if we are in an optional we need to capture dependencies
			if (not opt_operands.empty()) {
				// dependencies with parent group
				group_dependencies(group_patterns[group_patterns.size() - 2], group_patterns.back());
				// cartesian dependencies between optional patterns
				for (auto cur_op : group_patterns.back()) {
					for (auto opt_op : opt_operands.back()) {
						if (std::find(union_operands.back().begin(), union_operands.back().end(), opt_op) == union_operands.back().end()) {
							query->odg_.addConnection(cur_op, opt_op);
							query->odg_.addConnection(opt_op, cur_op);
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
			for (auto opt_ctx : optional_blocks.back()) {
				group_patterns.emplace_back();
				triples_blocks.emplace_back();
				optional_blocks.emplace_back();
				visitWellDesignedPattern(opt_ctx->groupGraphPattern()->groupGraphPatternSub(), {});
				union_operands.back().clear();
				optional_blocks.pop_back();
				triples_blocks.pop_back();
				group_patterns.pop_back();
			}
			union_operands.pop_back();
			opt_operands.pop_back();
			// prepare for the next union
			group_patterns.back().clear();
		} else {
			SparqlParser::GroupOrUnionGraphPatternContext *cur_gou_ctx = gou_ctxs.back();
			gou_ctxs.pop_back();
			size_t current_tbs = triples_blocks.back().size();
			size_t current_opts = optional_blocks.back().size();
			for (auto grp_ctx : cur_gou_ctx->groupGraphPattern()) {
				visitWellDesignedPattern(grp_ctx->groupGraphPatternSub(), gou_ctxs);
				triples_blocks.back().resize(current_tbs);
				optional_blocks.back().resize(current_opts);
			}
		}
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
				register_var(var);
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
			register_var(var);
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
				register_var(var);
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
			if (ctx->graphTerm()->iri())
				return rdf4cpp::rdf::Node(visitIri(ctx->graphTerm()->iri()).as<rdf4cpp::rdf::IRI>());
			else if (ctx->graphTerm()->blankNode())
				return rdf4cpp::rdf::Node(visitBlankNode(ctx->graphTerm()->blankNode()).as<rdf4cpp::rdf::query::Variable>());
			else if (ctx->graphTerm()->rdfLiteral())
				return rdf4cpp::rdf::Node(visitRdfLiteral(ctx->graphTerm()->rdfLiteral()).as<rdf4cpp::rdf::Literal>());
			else if (ctx->graphTerm()->booleanLiteral())
				return rdf4cpp::rdf::Node(visitBooleanLiteral(ctx->graphTerm()->booleanLiteral()).as<rdf4cpp::rdf::Literal>());
			else if (ctx->graphTerm()->numericLiteral())
				return rdf4cpp::rdf::Node(visitNumericLiteral(ctx->graphTerm()->numericLiteral()).as<rdf4cpp::rdf::Literal>());
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
			return rdf4cpp::rdf::IRI(query->prefixes_.at(predicate.substr(0, split)) + predicate.substr(split + 1));
		} catch (...) {
			throw std::out_of_range("Prefix " + predicate.substr(0, split) + " not declared.");
		}
	}

	antlrcpp::Any SelectAskQueryVisitor::visitBlankNode(SparqlParser::BlankNodeContext *ctx) {
		if (ctx->BLANK_NODE_LABEL())
			return rdf4cpp::rdf::query::Variable(ctx->BLANK_NODE_LABEL()->getText().substr(2), true);
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
		if (ctx->graphNodePath()->varOrTerm()) {
			rdf4cpp::rdf::Node obj = visitVarOrTerm(ctx->graphNodePath()->varOrTerm());
			if (obj.is_variable()) {
				auto var = rdf4cpp::rdf::query::Variable(obj);
				register_var(var);
				vars_in_scope.insert(var);
			}
			query->triple_patterns_.emplace_back(active_subject, active_predicate, obj);
			add_tp(query->triple_patterns_.back());
		} else {
			throw std::runtime_error("not supported");
		}
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitObject(SparqlParser::ObjectContext *ctx) {
		if (ctx->graphNode()->varOrTerm()) {
			rdf4cpp::rdf::Node obj = visitVarOrTerm(ctx->graphNode()->varOrTerm());
			if (obj.is_variable()) {
				auto var = rdf4cpp::rdf::query::Variable(obj);
				register_var(var);
				vars_in_scope.insert(var);
			}
			query->triple_patterns_.emplace_back(active_subject, active_predicate, obj);
			add_tp(query->triple_patterns_.back());
		} else {
			throw std::runtime_error("not supported");
		}
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitPath(SparqlParser::PathContext *ctx) {
		if (ctx->pathAlternative())
			return visitPathAlternative(ctx->pathAlternative());
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
		if (ctx->pathPrimary()->iri())
			return rdf4cpp::rdf::Node(visitIri(ctx->pathPrimary()->iri()).as<rdf4cpp::rdf::IRI>());
		else if (ctx->pathPrimary()->A())
			return rdf4cpp::rdf::Node(rdf4cpp::rdf::IRI("http://www.w3.org/1999/02/22-rdf-syntax-ns#type"));
		else if (ctx->pathPrimary()->NEGATION())
			throw std::runtime_error("Property paths are not supported yet");
		else
			return visitPath(ctx->pathPrimary()->path());
	}

	/* solution modifiers */

	antlrcpp::Any SelectAskQueryVisitor::visitGroupClause(SparqlParser::GroupClauseContext *ctx) {
		for (auto group_condition : ctx->groupCondition()) {
			if (group_condition->builtInCall()) {
				return nullptr; //built in call visitor
			} else if (group_condition->functionCall()) {
				return nullptr; //function call visitor
			} else if (group_condition->var()) {
				auto var = visitVar(group_condition->var()).as<rdf4cpp::rdf::query::Variable>();
				track_variable(var);
				query->grouping_keys_.push_back(std::make_unique<PrimaryVarExpression>(var, query->tracked_variables_[var]));
				vars_in_group_by.insert(var);
			} else if (group_condition->AS()) {
				return nullptr; // need to visit expression and track/assign alias
			} else {
				throw std::runtime_error("Unsupported GroupCondition");
			}
		}
		return nullptr;
	}

	/* expressions */

	antlrcpp::Any SelectAskQueryVisitor::visitExpression(SparqlParser::ExpressionContext *ctx) {
		if (auto base_ctx = dynamic_cast<SparqlParser::BaseExpressionContext *>(ctx); base_ctx) {
			auto res = std::move(visitPrimaryExpression(base_ctx->primaryExpression()).as<std::unique_ptr<Expression>>());
			return res;
		} else
			assert(false);
	}

	antlrcpp::Any SelectAskQueryVisitor::visitPrimaryExpression(SparqlParser::PrimaryExpressionContext *ctx) {
		std::unique_ptr<Expression> expr;
		if (ctx->var()) {
			auto var = visitVar(ctx->var()).as<rdf4cpp::rdf::query::Variable>();
			register_var(var);
			track_variable(var);
			vars_in_scope.insert(var); // todo: this needs to be changed when filters and minus are introduced
			expr = std::make_unique<PrimaryVarExpression>(var, query->tracked_variables_[var]);
		} else if (ctx->rdfLiteral()) {
			auto rdf_literal = visitRdfLiteral(ctx->rdfLiteral()).as<rdf4cpp::rdf::Literal>();
			expr = std::make_unique<PrimaryLiteralExpression>(rdf_literal);
		} else if (ctx->booleanLiteral()) {
			auto boolean_literal = visitBooleanLiteral(ctx->booleanLiteral()).as<rdf4cpp::rdf::Literal>();
			expr = std::make_unique<PrimaryLiteralExpression>(boolean_literal);
		} else if (ctx->numericLiteral()) {
			auto numeric_literal = visitBooleanLiteral(ctx->booleanLiteral()).as<rdf4cpp::rdf::Literal>();
			expr = std::make_unique<PrimaryLiteralExpression>(numeric_literal);
		} else if (ctx->builtInCall()) {
			if (ctx->builtInCall()->aggregate()) {
				expr = std::move(visitAggregate(ctx->builtInCall()->aggregate()).as<std::unique_ptr<Expression>>());
			}
		} else {
			expr = std::move(visitExpression(ctx->expression()).as<std::unique_ptr<Expression>>());
		}
		return expr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitAggregate(SparqlParser::AggregateContext *ctx) {
		query->contains_aggregates_ = true;
		std::unique_ptr<Expression> expr;
		std::unique_ptr<Expression> nested_expr;
		if (ctx->expression()) {
			nested_expr = std::move(visitExpression(ctx->expression()).as<std::unique_ptr<Expression>>());
			if (dynamic_cast<Aggregate *>(nested_expr.get())) {
				throw std::runtime_error("Nested aggregates are not allowed");
			}
		}
		if (ctx->DISTINCT()) {
			if (ctx->COUNT()) {
				if (ctx->ASTERISK()) {
					for (auto const &var : vars_in_scope) {
						track_variable(var);
					}
					expr = std::make_unique<CountStarDistinct>();
				} else {
					expr = std::make_unique<CountDistinct>(std::move(nested_expr));
				}
			} else {
				throw std::runtime_error("not supported");
			}
		} else {
			if (ctx->COUNT()) {
				if (ctx->ASTERISK()) {
					for (auto const &var : vars_in_scope) {
						track_variable(var);
					}
					expr = std::make_unique<CountStar>();
				} else {
					expr = std::make_unique<Count>(std::move(nested_expr));
				}
			} else {
				throw std::runtime_error("not supported");
			}
		}
		return expr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitRdfLiteral(SparqlParser::RdfLiteralContext *ctx) {
		std::string value = visitString(ctx->string());
		if (ctx->iri())
			return rdf4cpp::rdf::Literal(value, visitIri(ctx->iri()).as<rdf4cpp::rdf::IRI>());
		else if (ctx->LANGTAG())
			return rdf4cpp::rdf::Literal(value, ctx->LANGTAG()->getText().substr(1));
		else
			return rdf4cpp::rdf::Literal(value);
	}

	antlrcpp::Any SelectAskQueryVisitor::visitNumericLiteral(SparqlParser::NumericLiteralContext *ctx) {
		auto number = ctx->getText();
		if (ctx->numericLiteralPositive()) {
			if (ctx->numericLiteralPositive()->DECIMAL_POSITIVE())
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#decimal"));
			else if (ctx->numericLiteralPositive()->DOUBLE_POSITIVE())
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#double"));
			else
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"));
		} else if (ctx->numericLiteralPositive()) {
			if (ctx->numericLiteralNegative()->DECIMAL_NEGATIVE())
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#decimal"));
			else if (ctx->numericLiteralNegative()->DOUBLE_NEGATIVE())
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#double"));
			else
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"));
		} else {
			if (ctx->numericLiteralUnsigned()->DECIMAL())
				return rdf4cpp::rdf::Literal(number, rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#decimal"));
			else if (ctx->numericLiteralUnsigned()->DOUBLE())
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

	void SelectAskQueryVisitor::register_var(rdf4cpp::rdf::query::Variable const &var) {
		if (query->var_to_id_.contains(var))
			return;
		query->var_to_id_[var] = var_id;
		var_id++;
	}

	void SelectAskQueryVisitor::track_variable(rdf4cpp::rdf::query::Variable const &var) {
		if (query->tracked_variables_.contains(var))
			return;
		size_t pos = query->tracked_variables_.size();
		query->tracked_variables_[var] = pos;
	}

	void SelectAskQueryVisitor::register_alias(rdf4cpp::rdf::query::Variable const &var,
											   std::unique_ptr<Expression> expression) {
		query->aliases_[var] = std::move(expression);
	}

	void SelectAskQueryVisitor::add_tp(rdf4cpp::rdf::query::TriplePattern const &tp) {
		std::vector<char> var_ids{};
		for (auto const &node : tp) {
			if (not node.is_variable())
				continue;
			var_ids.push_back(query->var_to_id_[rdf4cpp::rdf::query::Variable(node)]);
		}
		// create new node in the operand dependency graph
		auto v_id = query->odg_.addOperand(var_ids);
		auto &gp = group_patterns.back();
		// iterate over the tps of the group and capture dependencies
		for (auto iter = gp.rbegin(); iter != gp.rend(); iter++) {
			std::set<char> done{};// only one edge per label between two nodes
			auto const &tp_vars = query->odg_.operandLabels(*iter);
			bool cart = true;
			for (auto const &var : var_ids) {
				for (auto const &tp_var : tp_vars) {
					if (var == tp_var) {
						cart = false;
						if (done.contains(var))
							continue;
						done.insert(var);
						query->odg_.addDependency(*iter, v_id, var);
						query->odg_.addDependency(v_id, *iter, var);
					}
				}
			}
			// the triple patterns do not share a variable --> cartesian join
			if (cart) {
				query->odg_.addDependency(*iter, v_id);
				query->odg_.addDependency(v_id, *iter);
			}
		}
		// add current tp/node to the active group pattern
		gp.push_back(v_id);
	}

	void SelectAskQueryVisitor::group_dependencies(std::vector<uint8_t> const &prev_group, std::vector<uint8_t> const &cur_group, bool bidirectional) {
		for (const auto &prev_tp : prev_group) {
			auto const &prev_labels = query->odg_.operandLabels(prev_tp);
			for (const auto &cur_tp : cur_group) {
				auto const &cur_labels = query->odg_.operandLabels(cur_tp);
				bool done = false;
				for (auto const &prev_label : prev_labels) {
					if (std::find(cur_labels.begin(), cur_labels.end(), prev_label) != cur_labels.end()) {
						query->odg_.addDependency(prev_tp, cur_tp, prev_label);
						if (bidirectional)
							query->odg_.addDependency(cur_tp, prev_tp, prev_label);
						done = true;
					}
				}
				if (not done) {
					query->odg_.addDependency(prev_tp, cur_tp);
					if (bidirectional)
						query->odg_.addDependency(cur_tp, prev_tp);
				}
			}
		}
	}

	void SelectAskQueryVisitor::group_connections(std::vector<uint8_t> const &prev_group,
												  std::vector<uint8_t> const &cur_group) {
		for (const auto &prev_tp : prev_group) {
			for (const auto &cur_tp : cur_group) {
				query->odg_.addConnection(prev_tp, cur_tp);
				query->odg_.addConnection(cur_tp, prev_tp);
			}
		}
	}

}// namespace Dice::sparql2tensor::parser::visitors