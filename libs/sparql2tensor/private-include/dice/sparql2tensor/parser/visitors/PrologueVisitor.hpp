#ifndef DICE_SPARQL_PROLOGUEVISITOR_HPP
#define DICE_SPARQL_PROLOGUEVISITOR_HPP

#include <rdf4cpp/rdf.hpp>

#include <SparqlParser/SparqlParserBaseVisitor.h>

#include <robin_hood.h>


namespace dice::sparql2tensor::parser::visitors {

	using namespace Dice::sparql_parser::base;

	class PrologueVisitor : public SparqlParserBaseVisitor {

		robin_hood::unordered_map<std::string, std::string> prefixes_;

	public:
		antlrcpp::Any visitPrologue(SparqlParser::PrologueContext *) override;

		antlrcpp::Any visitBaseDecl(SparqlParser::BaseDeclContext *) override;

		antlrcpp::Any visitPrefixDecl(SparqlParser::PrefixDeclContext *) override;
	};

}// namespace dice::sparql2tensor::parser::visitors

#endif//DICE_SPARQL_PROLOGUEVISITOR_HPP
