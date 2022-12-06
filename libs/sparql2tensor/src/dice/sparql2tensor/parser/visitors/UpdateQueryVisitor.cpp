#include "dice/sparql2tensor/parser/visitors/UpdateQueryVisitor.hpp"

namespace dice::sparql2tensor::parser::visitors {

	antlrcpp::Any UpdateQueryVisitor::visitUpdateCommand(SparqlParser::UpdateCommandContext *ctx) {
		if (ctx->update().size() > 1)
			throw std::runtime_error("Only single update operation is currently supported");
		auto update_ctx = ctx->update(0);
		visitUpdate(update_ctx);
		return nullptr;
	}

	antlrcpp::Any UpdateQueryVisitor::visitUpdate(SparqlParser::UpdateContext *ctx) {
		if (not ctx->deleteData())
			throw std::runtime_error("Only DELETE DATA and INSERT DATA are currently supported.");
		visitDeleteData(ctx->deleteData());
		return nullptr;
	}

	antlrcpp::Any UpdateQueryVisitor::visitDeleteData(SparqlParser::DeleteDataContext *ctx) {
		auto quad_data = visitQuadData(ctx->quadData()).as<std::vector<rdf_tensor::NonZeroEntry>>();
		assert(false); // this should not be called
		return nullptr;
	}

	antlrcpp::Any UpdateQueryVisitor::visitQuadData(SparqlParser::QuadDataContext *ctx) {
		auto quads_ctx = ctx->quads();
		std::vector<rdf_tensor::NonZeroEntry> entries{};
		std::vector<rdf4cpp::rdf::query::TriplePattern> triple_patterns;
		if (auto triples_template_ctx = quads_ctx->triplesTemplate(); triples_template_ctx) {
			auto current_tps = std::move(visitTriplesTemplate(triples_template_ctx, false).as<std::vector<rdf4cpp::rdf::query::TriplePattern>>());
			triple_patterns.insert(triple_patterns.end(), current_tps.begin(), current_tps.end());
		}
		if (not quads_ctx->quadsDetails().empty()) {
			throw std::runtime_error("Rule QuadsNotTriples is currently not supported.");
		}
		for (auto const &tp : triple_patterns) {
			entries.emplace_back(rdf_tensor::Key{tp.subject(), tp.predicate(), tp.object()});
		}
		return entries;
	}

	antlrcpp::Any UpdateQueryVisitor::visitTriplesTemplate(SparqlParser::TriplesTemplateContext *ctx, bool allow_vars) {
		std::vector<rdf4cpp::rdf::query::TriplePattern> triple_patterns{};
		for (auto triples_same_subject_ctx : ctx->triplesSameSubject()) {
			std::vector<rdf4cpp::rdf::query::TriplePattern> triples_same_subject = std::move(visitTriplesSameSubject(triples_same_subject_ctx,
																													 allow_vars));
			triple_patterns.insert(triple_patterns.end(), triples_same_subject.begin(), triples_same_subject.end());
		}
		return triple_patterns;
	}

	antlrcpp::Any UpdateQueryVisitor::visitTriplesSameSubject(SparqlParser::TriplesSameSubjectContext *ctx, bool allow_vars) {
		std::vector<rdf4cpp::rdf::query::TriplePattern> triples_same_subject{};
		if (ctx->triplesNode())
			throw std::runtime_error("Rule TriplesNode is currently not supported.");
		auto subject = visitVarOrTerm(ctx->varOrTerm()).as<rdf4cpp::rdf::Node>();
		if (not allow_vars and subject.is_variable())
			throw std::runtime_error("QuadData should not contain variables");
		auto property_list_not_empty_ctx = ctx->propertyListNotEmpty();
		for (size_t i = 0; i < property_list_not_empty_ctx->verb().size(); i++) {
			auto predicate = visitVerb(property_list_not_empty_ctx->verb(i)).as<rdf4cpp::rdf::Node>();
			if (not allow_vars and predicate.is_variable())
				throw std::runtime_error("QuadData should not contain variables");
			for (auto object_ctx : property_list_not_empty_ctx->objectList(i)->object()) {
				if (object_ctx->graphNode()->triplesNode())
					throw std::runtime_error("Rule TriplesNode is currently not supported.");
				auto object = visitVarOrTerm(object_ctx->graphNode()->varOrTerm()).as<rdf4cpp::rdf::Node>();
				if (not allow_vars and object.is_variable())
					throw std::runtime_error("QuadData should not contain variables");
				triples_same_subject.emplace_back(subject, predicate, object);
			}
		}
		return triples_same_subject;
	}

	antlrcpp::Any UpdateQueryVisitor::visitVarOrTerm(SparqlParser::VarOrTermContext *ctx) {
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

	antlrcpp::Any UpdateQueryVisitor::visitVerb(SparqlParser::VerbContext *ctx) {
		if (auto var_or_iri_ctx = ctx->varOrIRI(); var_or_iri_ctx) {
			if (auto var_ctx = var_or_iri_ctx->var(); var_ctx)
				return visitVar(var_ctx).as<rdf4cpp::rdf::Node>();
			return rdf4cpp::rdf::Node(visitIri(var_or_iri_ctx->iri()).as<rdf4cpp::rdf::IRI>());
		}
		return rdf4cpp::rdf::Node(rdf4cpp::rdf::IRI("http://www.w3.org/1999/02/22-rdf-syntax-ns#type"));
	}

	antlrcpp::Any UpdateQueryVisitor::visitIri(SparqlParser::IriContext *ctx) {
		if (ctx->IRIREF()) {
			auto iri = ctx->IRIREF()->getText();
			return rdf4cpp::rdf::IRI(iri.substr(1, iri.size() - 2));
		}
		std::string predicate = ctx->prefixedName()->PNAME_LN()->getText();
		std::size_t split = predicate.find(':');
		try {
			return rdf4cpp::rdf::IRI(query->prefixes.at(predicate.substr(0, split)) + predicate.substr(split + 1));
		} catch (...) {
			throw std::out_of_range("Prefix " + predicate.substr(0, split) + " not declared.");
		}
	}

	antlrcpp::Any UpdateQueryVisitor::visitBlankNode(SparqlParser::BlankNodeContext *ctx) {
		if (auto blank_node_label_ctx = ctx->BLANK_NODE_LABEL(); blank_node_label_ctx)
			return rdf4cpp::rdf::query::Variable(blank_node_label_ctx->getText().substr(2), true);
		else
			throw std::runtime_error("BlankNode ANON not supported.");
	}

	antlrcpp::Any UpdateQueryVisitor::visitVar(SparqlParser::VarContext *ctx) {
		return rdf4cpp::rdf::query::Variable(ctx->getText().substr(1));
	}

	antlrcpp::Any UpdateQueryVisitor::visitRdfLiteral(SparqlParser::RdfLiteralContext *ctx) {
		std::string value = visitString(ctx->string());
		if (auto iri_ctx = ctx->iri(); iri_ctx)
			return rdf4cpp::rdf::Literal(value, visitIri(iri_ctx).as<rdf4cpp::rdf::IRI>());
		else if (auto langtag_ctx = ctx->LANGTAG(); langtag_ctx)
			return rdf4cpp::rdf::Literal(value, langtag_ctx->getText().substr(1));
		else
			return rdf4cpp::rdf::Literal(value);
	}

	antlrcpp::Any UpdateQueryVisitor::visitNumericLiteral(SparqlParser::NumericLiteralContext *ctx) {
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

	antlrcpp::Any UpdateQueryVisitor::visitBooleanLiteral(SparqlParser::BooleanLiteralContext *ctx) {
		if (ctx->TRUE())
			return rdf4cpp::rdf::Literal("true", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean"));
		else
			return rdf4cpp::rdf::Literal("false", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean"));
	}

	antlrcpp::Any UpdateQueryVisitor::visitString(SparqlParser::StringContext *ctx) {
		std::string value = ctx->getText();
		if (ctx->STRING_LITERAL1() or ctx->STRING_LITERAL2())
			return value.substr(1, value.size() - 2);
		else
			return value.substr(3, value.size() - 6);
	}

}// namespace dice::sparql2tensor::parser::visitors