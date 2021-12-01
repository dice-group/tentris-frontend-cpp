#ifndef DICE_SPARQL_PROLOGUEVISITOR_HPP
#define DICE_SPARQL_PROLOGUEVISITOR_HPP

#include <SparqlParser/SparqlParserBaseVisitor.h>
#include <rdf4cpp/rdf.hpp>

#include "Dice/sparql/parser/ParsedSPARQL.hpp"

namespace Dice::sparql::parser::visitors {

	using namespace sparql_parser::base;

	class PrologueVisitor : public SparqlParserBaseVisitor {

		std::map<std::string, std::string> prefixes;

	public:
		antlrcpp::Any visitPrologue(SparqlParser::PrologueContext *) override;

		antlrcpp::Any visitBaseDecl(SparqlParser::BaseDeclContext *) override;

		antlrcpp::Any visitPrefixDecl(SparqlParser::PrefixDeclContext *) override;
	};

}// namespace Dice::sparql::parser::visitors

#endif//DICE_SPARQL_PROLOGUEVISITOR_HPP
