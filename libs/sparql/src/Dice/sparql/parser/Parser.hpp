#ifndef DICE_SPARQL_PARSER_HPP
#define DICE_SPARQL_PARSER_HPP

#include <SparqlLexer/SparqlLexer.h>
#include <SparqlParser/SparqlParser.h>

#include "ParsedSPARQL.hpp"
#include "visitors/PrologueVisitor.hpp"
#include "visitors/SelectQueryVisitor.hpp"

namespace Dice::sparql::parser {

	inline ParsedSPARQL parse_query(std::string const &query) {
		antlr4::ANTLRInputStream input(query);
		Dice::sparql_parser::base::SparqlLexer lexer(&input);
		antlr4::CommonTokenStream tokens(&lexer);
		Dice::sparql_parser::base::SparqlParser parser(&tokens);

		auto q_ctx = parser.query();
		if (not q_ctx->selectQuery())
			throw std::runtime_error("Only SELECT queries are supported currently.");

		ParsedSPARQL p_sparql{};
		if (q_ctx->prologue()) {
			visitors::PrologueVisitor p_visitor{};
			p_sparql.prefixes = p_visitor.visitPrologue(q_ctx->prologue()).as<std::map<std::string, std::string>>();
		}
		visitors::SelectQueryVisitor visitor{&p_sparql};
		visitor.visitSelectQuery(q_ctx->selectQuery());
		return p_sparql;
	}

}// namespace Dice::sparql::parser

#endif//DICE_SPARQL_PARSER_HPP
