#include "dice/sparql2tensor/parser/visitors/UpdateQueryVisitor.hpp"

// TODO: implement visitors for non DATA updates
namespace dice::sparql2tensor::parser::visitors {

	std::any UpdateQueryVisitor::visitUpdateCommand(SparqlParser::UpdateCommandContext *ctx) {
		if (ctx->update().size() > 1)
			throw std::runtime_error("Only single update operation is currently supported");
		auto update_ctx = ctx->update(0);
		visitUpdate(update_ctx);
		return nullptr;
	}

	std::any UpdateQueryVisitor::visitUpdate(SparqlParser::UpdateContext *ctx) {
		if (not ctx->deleteData())
			throw std::runtime_error("Only DELETE DATA and INSERT DATA are currently supported.");
		visitDeleteData(ctx->deleteData());
		return nullptr;
	}

	std::any UpdateQueryVisitor::visitDeleteData(SparqlParser::DeleteDataContext *ctx) {
		auto quad_data = std::any_cast<std::vector<rdf_tensor::NonZeroEntry>>(visitQuadData(ctx->quadData()));
		assert(false); // this should not be called
		return nullptr;
	}

	std::any UpdateQueryVisitor::visitQuadData(SparqlParser::QuadDataContext *ctx) {
		auto quads_ctx = ctx->quads();
		std::vector<rdf_tensor::NonZeroEntry> entries{};
		std::vector<rdf4cpp::rdf::query::TriplePattern> triple_patterns;
		if (auto triples_template_ctx = quads_ctx->triplesTemplate(); triples_template_ctx) {
			auto current_tps = std::any_cast<std::vector<rdf4cpp::rdf::query::TriplePattern>>(visitTriplesTemplate(triples_template_ctx, false));
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

	std::any UpdateQueryVisitor::visitTriplesTemplate(SparqlParser::TriplesTemplateContext *ctx, bool allow_vars) {
		std::vector<rdf4cpp::rdf::query::TriplePattern> triple_patterns{};
		for (auto triples_same_subject_ctx : ctx->triplesSameSubject()) {
			auto triples_same_subject = std::any_cast<std::vector<rdf4cpp::rdf::query::TriplePattern>>(visitTriplesSameSubject(triples_same_subject_ctx, allow_vars));
			triple_patterns.insert(triple_patterns.end(), triples_same_subject.begin(), triples_same_subject.end());
		}
		return triple_patterns;
	}

	std::any UpdateQueryVisitor::visitTriplesSameSubject(SparqlParser::TriplesSameSubjectContext *ctx, bool allow_vars) {
		std::vector<rdf4cpp::rdf::query::TriplePattern> triples_same_subject{};
		if (ctx->triplesNode())
			throw std::runtime_error("Rule TriplesNode is currently not supported.");
		auto subject = std::any_cast<rdf4cpp::rdf::Node>(visitVarOrTerm(ctx->varOrTerm()));
		if (not allow_vars and subject.is_variable())
			throw std::runtime_error("QuadData should not contain variables");
		auto property_list_not_empty_ctx = ctx->propertyListNotEmpty();
		for (size_t i = 0; i < property_list_not_empty_ctx->verb().size(); i++) {
			auto predicate = std::any_cast<rdf4cpp::rdf::Node>(visitVerb(property_list_not_empty_ctx->verb(i)));
			if (not allow_vars and predicate.is_variable())
				throw std::runtime_error("QuadData should not contain variables");
			for (auto object_ctx : property_list_not_empty_ctx->objectList(i)->object()) {
				if (object_ctx->graphNode()->triplesNode())
					throw std::runtime_error("Rule TriplesNode is currently not supported.");
				auto object = std::any_cast<rdf4cpp::rdf::Node>(visitVarOrTerm(object_ctx->graphNode()->varOrTerm()));
				if (not allow_vars and object.is_variable())
					throw std::runtime_error("QuadData should not contain variables");
				triples_same_subject.emplace_back(subject, predicate, object);
			}
		}
		return triples_same_subject;
	}

	std::any UpdateQueryVisitor::visitVarOrTerm(SparqlParser::VarOrTermContext *ctx) {
		if (ctx->var()) {
			return static_cast<rdf4cpp::rdf::Node>(std::any_cast<rdf4cpp::rdf::query::Variable>(visitVar(ctx->var())));
		} else {
			if (auto iri_ctx = ctx->graphTerm()->iri())
				return static_cast<rdf4cpp::rdf::Node>(std::any_cast<rdf4cpp::rdf::IRI>(visitIri(iri_ctx)));
			else if (auto blank_node_ctx = ctx->graphTerm()->blankNode(); blank_node_ctx)
				return static_cast<rdf4cpp::rdf::Node>(std::any_cast<rdf4cpp::rdf::query::Variable>(visitBlankNode(blank_node_ctx)));
			else if (auto rdf_literal_ctx = ctx->graphTerm()->rdfLiteral(); rdf_literal_ctx)
				return static_cast<rdf4cpp::rdf::Node>(std::any_cast<rdf4cpp::rdf::Literal>(visitRdfLiteral(rdf_literal_ctx)));
			else if (auto boolean_literal_ctx = ctx->graphTerm()->booleanLiteral(); boolean_literal_ctx)
				return static_cast<rdf4cpp::rdf::Node>(std::any_cast<rdf4cpp::rdf::Literal>(visitBooleanLiteral(boolean_literal_ctx)));
			else if (auto numberic_literal_ctx = ctx->graphTerm()->numericLiteral(); numberic_literal_ctx)
				return static_cast<rdf4cpp::rdf::Node>(std::any_cast<rdf4cpp::rdf::Literal>(visitNumericLiteral(numberic_literal_ctx)));
			else
				throw std::runtime_error("RDF collections are not supported yet.");
		}
	}

	std::any UpdateQueryVisitor::visitVerb(SparqlParser::VerbContext *ctx) {
		if (auto var_or_iri_ctx = ctx->varOrIRI(); var_or_iri_ctx) {
			if (auto var_ctx = var_or_iri_ctx->var(); var_ctx)
				return std::any_cast<rdf4cpp::rdf::Node>(visitVar(var_ctx));
			return static_cast<rdf4cpp::rdf::Node>(std::any_cast<rdf4cpp::rdf::IRI>(visitIri(var_or_iri_ctx->iri())));
		}
		return static_cast<rdf4cpp::rdf::Node>(rdf4cpp::rdf::IRI("http://www.w3.org/1999/02/22-rdf-syntax-ns#type"));
	}

	std::any UpdateQueryVisitor::visitIri(SparqlParser::IriContext *ctx) {
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

	std::any UpdateQueryVisitor::visitBlankNode(SparqlParser::BlankNodeContext *ctx) {
		if (auto blank_node_label_ctx = ctx->BLANK_NODE_LABEL(); blank_node_label_ctx)
			return rdf4cpp::rdf::query::Variable(blank_node_label_ctx->getText().substr(2), true);
		else
			throw std::runtime_error("BlankNode ANON not supported.");
	}

	std::any UpdateQueryVisitor::visitVar(SparqlParser::VarContext *ctx) {
		return rdf4cpp::rdf::query::Variable(ctx->getText().substr(1));
	}

	std::any UpdateQueryVisitor::visitRdfLiteral(SparqlParser::RdfLiteralContext *ctx) {
		std::string value = std::any_cast<std::string>(visitString(ctx->string()));
		if (auto iri_ctx = ctx->iri(); iri_ctx)
			return rdf4cpp::rdf::Literal{value, std::any_cast<rdf4cpp::rdf::IRI>(visitIri(iri_ctx))};
		else if (auto langtag_ctx = ctx->LANGTAG(); langtag_ctx)
			return rdf4cpp::rdf::Literal{value, langtag_ctx->getText().substr(1)};
		else
			return rdf4cpp::rdf::Literal{value};
	}

	std::any UpdateQueryVisitor::visitNumericLiteral(SparqlParser::NumericLiteralContext *ctx) {
		auto number = ctx->getText();
		if (auto pos_literal_ctx = ctx->numericLiteralPositive(); pos_literal_ctx) {
			if (pos_literal_ctx->DECIMAL_POSITIVE())
				return rdf4cpp::rdf::Literal{number, rdf4cpp::rdf::datatypes::xsd::Decimal::identifier};
			else if (pos_literal_ctx->DOUBLE_POSITIVE())
				return rdf4cpp::rdf::Literal{number, rdf4cpp::rdf::datatypes::xsd::Double::identifier};
			else
				return rdf4cpp::rdf::Literal{number, rdf4cpp::rdf::datatypes::xsd::Integer::identifier};
		} else if (auto neg_literal_ctx = ctx->numericLiteralNegative(); neg_literal_ctx) {
			if (neg_literal_ctx->DECIMAL_NEGATIVE())
				return rdf4cpp::rdf::Literal{number, rdf4cpp::rdf::datatypes::xsd::Decimal::identifier};
			else if (neg_literal_ctx->DOUBLE_NEGATIVE())
				return rdf4cpp::rdf::Literal{number, rdf4cpp::rdf::datatypes::xsd::Double::identifier};
			else
				return rdf4cpp::rdf::Literal{number, rdf4cpp::rdf::datatypes::xsd::Integer::identifier};
		} else {
			auto unsigned_literal_ctx = ctx->numericLiteralUnsigned();
			if (unsigned_literal_ctx->DECIMAL())
				return rdf4cpp::rdf::Literal{number, rdf4cpp::rdf::datatypes::xsd::Decimal::identifier};
			else if (unsigned_literal_ctx->DOUBLE())
				return rdf4cpp::rdf::Literal{number, rdf4cpp::rdf::datatypes::xsd::Double::identifier};
			else
				return rdf4cpp::rdf::Literal{number, rdf4cpp::rdf::datatypes::xsd::Integer::identifier};
		}
	}

	std::any UpdateQueryVisitor::visitBooleanLiteral(SparqlParser::BooleanLiteralContext *ctx) {
		return rdf4cpp::rdf::Literal::make<rdf4cpp::rdf::datatypes::xsd::Boolean>(ctx->TRUE() != nullptr);
	}

	std::any UpdateQueryVisitor::visitString(SparqlParser::StringContext *ctx) {
		std::string value = ctx->getText();
		if (ctx->STRING_LITERAL1() or ctx->STRING_LITERAL2())
			return value.substr(1, value.size() - 2);
		else
			return value.substr(3, value.size() - 6);
	}

}// namespace dice::sparql2tensor::parser::visitors