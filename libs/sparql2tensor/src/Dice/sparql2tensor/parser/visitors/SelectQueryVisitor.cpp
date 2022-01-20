#include "Dice/sparql2tensor/parser/visitors/SelectQueryVisitor.hpp"

namespace Dice::sparql2tensor::parser::visitors {

	antlrcpp::Any SelectQueryVisitor::visitSelectQuery(SparqlParser::SelectQueryContext *ctx) {
		if (ctx->whereClause())
			visitWhereClause(ctx->whereClause());
		else
			throw std::runtime_error("Query does not contain a WHERE clause");
		visitSelectClause(ctx->selectClause());
		return nullptr;
	}

	antlrcpp::Any SelectQueryVisitor::visitSelectClause(SparqlParser::SelectClauseContext *ctx) {
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

	antlrcpp::Any SelectQueryVisitor::visitWhereClause(SparqlParser::WhereClauseContext *ctx) {
		visitGroupGraphPattern(ctx->groupGraphPattern());
		return nullptr;
	}

	antlrcpp::Any SelectQueryVisitor::visitGroupGraphPattern(SparqlParser::GroupGraphPatternContext *ctx) {
		if (ctx->subSelect())
			throw std::runtime_error("Subqueries are not supported yet");
		else if (ctx->groupGraphPatternSub())
			visitGroupGraphPatternSub(ctx->groupGraphPatternSub());
		else
			throw std::runtime_error("Malformed query");
		return nullptr;
	}

	antlrcpp::Any SelectQueryVisitor::visitGroupGraphPatternSub(SparqlParser::GroupGraphPatternSubContext *ctx) {
		if (ctx->triplesBlock())
			visitTriplesBlock(ctx->triplesBlock());
		for (auto sub_ctx : ctx->groupGraphPatternSubList()) {
			if (sub_ctx->graphPatternNotTriples())
				visitGraphPatternNotTriples(sub_ctx->graphPatternNotTriples());
			if (sub_ctx->triplesBlock())
				visitTriplesBlock(sub_ctx->triplesBlock());
		}
		return nullptr;
	}

	antlrcpp::Any SelectQueryVisitor::visitTriplesBlock(SparqlParser::TriplesBlockContext *ctx) {
		for (auto sub_ctx : ctx->triplesSameSubjectPath())
			visitTriplesSameSubjectPath(sub_ctx);
		return nullptr;
	}

	antlrcpp::Any SelectQueryVisitor::visitTriplesSameSubjectPath(SparqlParser::TriplesSameSubjectPathContext *ctx) {
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

	antlrcpp::Any SelectQueryVisitor::visitPropertyListPathNotEmpty(SparqlParser::PropertyListPathNotEmptyContext *ctx) {
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

	antlrcpp::Any SelectQueryVisitor::visitTriplesNodePath([[maybe_unused]] SparqlParser::TriplesNodePathContext *ctx) {
		return nullptr;
	}
	antlrcpp::Any SelectQueryVisitor::visitBlankNodePropertyListPath([[maybe_unused]] SparqlParser::BlankNodePropertyListPathContext *ctx) {
		return nullptr;
	}
	antlrcpp::Any SelectQueryVisitor::visitGraphPatternNotTriples([[maybe_unused]] SparqlParser::GraphPatternNotTriplesContext *ctx) {
		return nullptr;
	}
	antlrcpp::Any SelectQueryVisitor::visitOptionalGraphPattern([[maybe_unused]] SparqlParser::OptionalGraphPatternContext *ctx) {
		return nullptr;
	}
	antlrcpp::Any SelectQueryVisitor::visitGroupOrUnionGraphPattern([[maybe_unused]] SparqlParser::GroupOrUnionGraphPatternContext *ctx) {
		return nullptr;
	}
	antlrcpp::Any SelectQueryVisitor::visitMinusGraphPattern([[maybe_unused]] SparqlParser::MinusGraphPatternContext *ctx) {
		return nullptr;
	}

	antlrcpp::Any SelectQueryVisitor::visitVarOrTerm(SparqlParser::VarOrTermContext *ctx) {
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

	antlrcpp::Any SelectQueryVisitor::visitIri(SparqlParser::IriContext *ctx) {
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

	antlrcpp::Any SelectQueryVisitor::visitBlankNode(SparqlParser::BlankNodeContext *ctx) {
		if (ctx->BLANK_NODE_LABEL())
			return rdf4cpp::rdf::query::Variable(ctx->BLANK_NODE_LABEL()->getText().substr(2), true);
		else
			throw std::runtime_error("BlankNode ANON not supported.");
	}

	antlrcpp::Any SelectQueryVisitor::visitVar(SparqlParser::VarContext *ctx) {
		return rdf4cpp::rdf::query::Variable(ctx->getText().substr(1));
	}

	antlrcpp::Any SelectQueryVisitor::visitObjectListPath(SparqlParser::ObjectListPathContext *ctx) {
		for (auto objp_ctx : ctx->objectPath())
			visitObjectPath(objp_ctx);
		return nullptr;
	}

	antlrcpp::Any SelectQueryVisitor::visitObjectList(SparqlParser::ObjectListContext *ctx) {
		for (auto obj_ctx : ctx->object())
			visitObject(obj_ctx);
		return nullptr;
	}

	antlrcpp::Any SelectQueryVisitor::visitObjectPath(SparqlParser::ObjectPathContext *ctx) {
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

	antlrcpp::Any SelectQueryVisitor::visitObject(SparqlParser::ObjectContext *ctx) {
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

	antlrcpp::Any SelectQueryVisitor::visitPath(SparqlParser::PathContext *ctx) {
		if (ctx->pathAlternative())
			return visitPathAlternative(ctx->pathAlternative());
		else
			throw std::runtime_error("Malformed query.");
	}

	antlrcpp::Any SelectQueryVisitor::visitPathAlternative(SparqlParser::PathAlternativeContext *ctx) {
		if (ctx->pathSequence().size() > 1)
			throw std::runtime_error("Property paths are not supported yet");
		return visitPathSequence(ctx->pathSequence(0));
	}

	antlrcpp::Any SelectQueryVisitor::visitPathSequence(SparqlParser::PathSequenceContext *ctx) {
		if (ctx->pathEltOrInverse().size() > 1)
			throw std::runtime_error("Property paths are not supported yet");
		return visitPathEltOrInverse(ctx->pathEltOrInverse(0));
	}

	antlrcpp::Any SelectQueryVisitor::visitPathEltOrInverse(SparqlParser::PathEltOrInverseContext *ctx) {
		if (ctx->INVERSE())
			throw std::runtime_error("Property paths are not supported yet");
		return visitPathElt(ctx->pathElt());
	}

	antlrcpp::Any SelectQueryVisitor::visitPathElt(SparqlParser::PathEltContext *ctx) {
		if (ctx->pathPrimary()->iri())
			return rdf4cpp::rdf::Node(visitIri(ctx->pathPrimary()->iri()).as<rdf4cpp::rdf::IRI>());
		else if (ctx->pathPrimary()->A())
			return rdf4cpp::rdf::Node(rdf4cpp::rdf::IRI("http://www.w3.org/1999/02/22-rdf-syntax-ns#type"));
		else if (ctx->pathPrimary()->NEGATION())
			throw std::runtime_error("Property paths are not supported yet");
		else
			return visitPath(ctx->pathPrimary()->path());
	}

	antlrcpp::Any SelectQueryVisitor::visitRdfLiteral(SparqlParser::RdfLiteralContext *ctx) {
		std::string value = visitString(ctx->string());
		if (ctx->iri())
			return rdf4cpp::rdf::Literal(value, visitIri(ctx->iri()).as<rdf4cpp::rdf::IRI>());
		else if (ctx->LANGTAG())
			return rdf4cpp::rdf::Literal(value, ctx->LANGTAG()->getText());
		else
			return rdf4cpp::rdf::Literal(value);
	}

	antlrcpp::Any SelectQueryVisitor::visitNumericLiteral(SparqlParser::NumericLiteralContext *ctx) {
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

	antlrcpp::Any SelectQueryVisitor::visitBooleanLiteral(SparqlParser::BooleanLiteralContext *ctx) {
		if (ctx->TRUE())
			return rdf4cpp::rdf::Literal("true", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean"));
		else
			return rdf4cpp::rdf::Literal("false", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean"));
	}

	antlrcpp::Any SelectQueryVisitor::visitString(SparqlParser::StringContext *ctx) {
		std::string value = ctx->getText();
		if (ctx->STRING_LITERAL1() or ctx->STRING_LITERAL2())
			return value.substr(1, value.size() - 2);
		else
			return value.substr(3, value.size() - 6);
	}

	void SelectQueryVisitor::register_var(rdf4cpp::rdf::query::Variable const &var) {
		if (query->var_to_id_.contains(var))
			return;
		query->var_to_id_[var] = var_id;
		var_id++;
	}

	void SelectQueryVisitor::add_tp(rdf4cpp::rdf::query::TriplePattern const &tp) {
		std::vector<char> var_ids{};
		for (auto const &node : tp) {
			if (not node.is_variable())
				continue;
			var_ids.push_back(query->var_to_id_[rdf4cpp::rdf::query::Variable(node)]);
		}
	}

}// namespace Dice::sparql2tensor::parser::visitors