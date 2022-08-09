#ifndef DICE_SPARQL_UPDATEQUERYVISITOR_HPP
#define DICE_SPARQL_UPDATEQUERYVISITOR_HPP

#include <rdf4cpp/rdf.hpp>

#include <SparqlParser/SparqlParserBaseVisitor.h>

#include "dice/sparql2tensor/UPDATEQuery.hpp"

namespace dice::sparql2tensor::parser::visitors {

	using namespace Dice::sparql_parser::base;

	class UpdateQueryVisitor : public SparqlParserBaseVisitor {
	private:
		UPDATEQuery *const query;

	public:
		UpdateQueryVisitor() = delete;

		explicit UpdateQueryVisitor(UPDATEQuery *q) : query{q} {}

		antlrcpp::Any visitUpdateCommand(SparqlParser::UpdateCommandContext *ctx) override;

		antlrcpp::Any visitUpdate(SparqlParser::UpdateContext *ctx) override;

		antlrcpp::Any visitDeleteData(SparqlParser::DeleteDataContext *ctx) override;

		antlrcpp::Any visitQuadData(SparqlParser::QuadDataContext *ctx) override;

		antlrcpp::Any visitTriplesTemplate(SparqlParser::TriplesTemplateContext *ctx, bool allow_vars = true);

		antlrcpp::Any visitTriplesSameSubject(SparqlParser::TriplesSameSubjectContext *ctx, bool allow_vars = true);

		antlrcpp::Any visitVarOrTerm(SparqlParser::VarOrTermContext *) override;

		antlrcpp::Any visitVerb(SparqlParser::VerbContext *ctx) override;

		antlrcpp::Any visitIri(SparqlParser::IriContext *) override;

		antlrcpp::Any visitBlankNode(SparqlParser::BlankNodeContext *) override;

		antlrcpp::Any visitVar(SparqlParser::VarContext *) override;

		antlrcpp::Any visitRdfLiteral(SparqlParser::RdfLiteralContext *) override;

		antlrcpp::Any visitNumericLiteral(SparqlParser::NumericLiteralContext *) override;

		antlrcpp::Any visitBooleanLiteral(SparqlParser::BooleanLiteralContext *) override;

		antlrcpp::Any visitString(SparqlParser::StringContext *) override;
	};

}// namespace dice::sparql2tensor::parser::visitors

#endif//DICE_SPARQL_UPDATEQUERYVISITOR_HPP
