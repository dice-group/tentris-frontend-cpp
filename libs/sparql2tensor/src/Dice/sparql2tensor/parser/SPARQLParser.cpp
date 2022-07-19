#include "SPARQLParser.hpp"

#include "Dice/sparql2tensor/parser/visitors/PrologueVisitor.hpp"
#include "Dice/sparql2tensor/parser/visitors/SelectAskQueryVisitor.hpp"

#include <SparqlLexer/SparqlLexer.h>
#include <SparqlParser/SparqlParser.h>

namespace Dice::sparql2tensor::parser {

	SPARQLQuery SPARQLParser::parse_query(const std::string &sparql_query_str,
										  const triple_store::TripleStore &triple_store) {
		SPARQLQuery sparql_query;
		// prepare antlr4 parser
		antlr4::ANTLRInputStream input(sparql_query_str);
		Dice::sparql_parser::base::SparqlLexer lexer(&input);
		antlr4::CommonTokenStream tokens(&lexer);
		Dice::sparql_parser::base::SparqlParser parser(&tokens);
		// check if the provided string is a QueryUnit (https://www.w3.org/TR/sparql11-query/#rQueryUnit)
		auto query_ctx = parser.query();
		if (not query_ctx)
			throw std::runtime_error("The provided query is not a QueryUnit");
		// parse the prefixes
		if (auto prologue_ctx = query_ctx->prologue(); prologue_ctx) {
			visitors::PrologueVisitor prologue_visitor{};
			auto prefixes = prologue_visitor.visitPrologue(query_ctx->prologue());
			sparql_query.set_prefixes(std::move(prefixes));
		}
		// parse the query
		visitors::SelectAskQueryVisitor select_ask_visitor{&sparql_query, triple_store};
		if (auto ask_ctx = query_ctx->askQuery(); ask_ctx)
			select_ask_visitor.visitAskQuery(ask_ctx);
		else if (auto select_ctx = query_ctx->selectQuery(); select_ctx)
			select_ask_visitor.visitSelectQuery(select_ctx);
		else
			throw std::runtime_error("Only SELECT and ASK queries are currently supported.");
		return sparql_query;
	}

	void SPARQLParser::parse_update([[maybe_unused]] const std::string &sparql_query_str,
									[[maybe_unused]] const triple_store::TripleStore &triple_store) {
		throw std::runtime_error("not implemented");
	}

}// namespace Dice::sparql2tensor::parser