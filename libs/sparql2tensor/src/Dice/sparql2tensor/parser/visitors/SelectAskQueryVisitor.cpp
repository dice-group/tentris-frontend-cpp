#include "Dice/sparql2tensor/parser/visitors/SelectAskQueryVisitor.hpp"

namespace Dice::sparql2tensor::parser::visitors {

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
		visitSelectClause(ctx->selectClause());
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitSelectClause(SparqlParser::SelectClauseContext *ctx) {
		if (ctx->selectModifier()) {
			if (ctx->selectModifier()->DISTINCT())
				query->distinct_ = true;
		}
		if (ctx->ASTERISK()) {
			query->project_all_variables_ = true;
			std::unordered_set<rdf4cpp::rdf::query::Variable> seen_vars;
			// set all non-anonymous variables from the triple patterns
			for (auto const &tp : query->triple_patterns_) {
				for (auto const &node : tp) {
					if (node.is_variable()) {
						auto var = (rdf4cpp::rdf::query::Variable) node;
						if (not var.is_anonymous()) {
							auto [_, was_new] = seen_vars.insert(var);
							if (was_new)
								query->projected_variables_.push_back(var);
						}
					}
				}
			}
		} else {
			for (auto sel_ctx : ctx->selectVariables()) {
				if (sel_ctx->var()) {
					auto var = visitVar(sel_ctx->var()).as<rdf4cpp::rdf::query::Variable>();
					register_var(var);
					query->projected_variables_.push_back(var);
				} else {
					throw std::runtime_error("Expressions in SELECT clause are not supported yet.");
				}
			}
		}
		if (query->projected_variables_.empty())
			throw std::runtime_error("At least one variable should be projected.");
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitWhereClause(SparqlParser::WhereClauseContext *ctx) {
		group_patterns.emplace_back();
		visitGroupGraphPattern(ctx->groupGraphPattern());
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
		if (ctx->triplesBlock())
			visitTriplesBlock(ctx->triplesBlock());
		for (auto sub_ctx : ctx->groupGraphPatternSubList()) {
			if (sub_ctx->graphPatternNotTriples())
				visitGraphPatternNotTriples(sub_ctx->graphPatternNotTriples());
			if (sub_ctx->triplesBlock())
				visitTriplesBlock(sub_ctx->triplesBlock());
		}
		last_group_pattern.clear();
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
			if (active_subject.is_variable())
				register_var(rdf4cpp::rdf::query::Variable(active_subject));
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
				active_predicate = rdf4cpp::rdf::Node(var);
			}
			if (not prop_ctx->objectList())
				throw std::runtime_error("Triple requires at least one object");
			visitObjectList(prop_ctx->objectList());
		}
		return nullptr;
	}

	antlrcpp::Any SelectAskQueryVisitor::visitTriplesNodePath([[maybe_unused]] SparqlParser::TriplesNodePathContext *ctx) {
		return nullptr;
	}
	antlrcpp::Any SelectAskQueryVisitor::visitBlankNodePropertyListPath([[maybe_unused]] SparqlParser::BlankNodePropertyListPathContext *ctx) {
		return nullptr;
	}
	antlrcpp::Any SelectAskQueryVisitor::visitGraphPatternNotTriples([[maybe_unused]] SparqlParser::GraphPatternNotTriplesContext *ctx) {
		if (ctx->groupOrUnionGraphPattern())
			visitGroupOrUnionGraphPattern(ctx->groupOrUnionGraphPattern());
		if (ctx->optionalGraphPattern())
			visitOptionalGraphPattern(ctx->optionalGraphPattern());
		return nullptr;
	}
	antlrcpp::Any SelectAskQueryVisitor::visitOptionalGraphPattern([[maybe_unused]] SparqlParser::OptionalGraphPatternContext *ctx) {
		if (ctx->groupGraphPattern()) {
			group_patterns.emplace_back();
			visitGroupGraphPattern(ctx->groupGraphPattern());
			group_dependencies(group_patterns[group_patterns.size() - 2], group_patterns[group_patterns.size() - 1], false);
			if (not last_group_pattern.empty())
				group_connections(last_group_pattern, group_patterns.back());
			last_group_pattern = std::move(group_patterns.back());
			group_patterns.pop_back();
		}
		return nullptr;
	}
	antlrcpp::Any SelectAskQueryVisitor::visitGroupOrUnionGraphPattern([[maybe_unused]] SparqlParser::GroupOrUnionGraphPatternContext *ctx) {
		assert(not ctx->groupGraphPattern().empty());
		// if union is within optional, treat it as a left join
		bool in_opt = ctx->parent->getText().find("OPTIONAL");
		bool join = not in_opt and ctx->groupGraphPattern().size() == 1;
		for (auto ggp : ctx->groupGraphPattern()) {
			group_patterns.emplace_back();
			visitGroupGraphPattern(ggp);
			if (group_patterns.size() > 1)
				group_dependencies(group_patterns[group_patterns.size() - 2], group_patterns[group_patterns.size() - 1], join, not in_opt);
			group_patterns.pop_back();
		}
		return nullptr;
	}
	antlrcpp::Any SelectAskQueryVisitor::visitMinusGraphPattern([[maybe_unused]] SparqlParser::MinusGraphPatternContext *ctx) {
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
			if (obj.is_variable())
				register_var(rdf4cpp::rdf::query::Variable(obj));
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
			if (obj.is_variable())
				register_var(rdf4cpp::rdf::query::Variable(obj));
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

	antlrcpp::Any SelectAskQueryVisitor::visitRdfLiteral(SparqlParser::RdfLiteralContext *ctx) {
		std::string value = visitString(ctx->string());
		if (ctx->iri())
			return rdf4cpp::rdf::Literal(value, visitIri(ctx->iri()).as<rdf4cpp::rdf::IRI>());
		else if (ctx->LANGTAG())
			return rdf4cpp::rdf::Literal(value, ctx->LANGTAG()->getText());
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
		std::set<char> done{};
		// iterate over the tps of the group and capture dependencies
		for (auto iter = gp.rbegin(); iter != gp.rend(); iter++) {
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

	void SelectAskQueryVisitor::group_dependencies(std::vector<uint8_t> const &prev_group,
												   std::vector<uint8_t> const &cur_group,
												   bool bidirectional,
												   bool is_union) {
		for (const auto &prev_tp : prev_group) {
			auto const &prev_labels = query->odg_.operandLabels(prev_tp);
			for (const auto &cur_tp : cur_group) {
				auto const &cur_labels = query->odg_.operandLabels(cur_tp);
				for (auto const &prev_label : prev_labels) {
					if (std::find(cur_labels.begin(), cur_labels.end(), prev_label) != cur_labels.end()) {
						query->odg_.addDependency(prev_tp, cur_tp, prev_label, is_union);
						if (bidirectional)
							query->odg_.addDependency(cur_tp, prev_tp, prev_label, is_union);
					} else {
						query->odg_.addDependency(prev_tp, cur_tp, '\0', is_union);
						if (bidirectional)
							query->odg_.addDependency(cur_tp, prev_tp, '\0', is_union);
					}
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