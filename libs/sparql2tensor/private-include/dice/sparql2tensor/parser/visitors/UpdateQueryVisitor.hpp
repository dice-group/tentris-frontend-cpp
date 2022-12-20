#ifndef DICE_SPARQL_UPDATEQUERYVISITOR_HPP
#define DICE_SPARQL_UPDATEQUERYVISITOR_HPP

#include <rdf4cpp/rdf.hpp>

#include <SparqlParser/SparqlParserBaseVisitor.h>

#include "dice/sparql2tensor/UPDATEQuery.hpp"

namespace dice::sparql2tensor::parser::visitors {

	using namespace dice::sparql_parser::base;

	// TODO: implement visitors for non DATA updates
	// NOTE: currently unused, will be later used to support non DATA updates
	class UpdateQueryVisitor : public SparqlParserBaseVisitor {
	private:
		UPDATEQuery *const query;

	public:
		UpdateQueryVisitor() = delete;

		explicit UpdateQueryVisitor(UPDATEQuery &q) : query{&q} {}

		std::any visitUpdateCommand(SparqlParser::UpdateCommandContext *ctx) override;

		std::any visitUpdate(SparqlParser::UpdateContext *ctx) override;

		std::any visitDeleteData(SparqlParser::DeleteDataContext *ctx) override;

		std::any visitQuadData(SparqlParser::QuadDataContext *ctx) override;

		std::any visitTriplesTemplate(SparqlParser::TriplesTemplateContext *ctx) override;
		std::any visitTriplesTemplate(SparqlParser::TriplesTemplateContext *ctx, bool allow_vars);

		std::any visitTriplesSameSubject(SparqlParser::TriplesSameSubjectContext *ctx) override;
		std::any visitTriplesSameSubject(SparqlParser::TriplesSameSubjectContext *ctx, bool allow_vars);

		std::any visitVarOrTerm(SparqlParser::VarOrTermContext *) override;

		std::any visitVerb(SparqlParser::VerbContext *ctx) override;

		std::any visitIri(SparqlParser::IriContext *) override;

		std::any visitBlankNode(SparqlParser::BlankNodeContext *) override;

		std::any visitVar(SparqlParser::VarContext *) override;

		std::any visitRdfLiteral(SparqlParser::RdfLiteralContext *) override;

		std::any visitNumericLiteral(SparqlParser::NumericLiteralContext *) override;

		std::any visitBooleanLiteral(SparqlParser::BooleanLiteralContext *) override;

		std::any visitString(SparqlParser::StringContext *) override;
	};

}// namespace dice::sparql2tensor::parser::visitors

#endif//DICE_SPARQL_UPDATEQUERYVISITOR_HPP
