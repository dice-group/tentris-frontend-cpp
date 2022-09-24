#include "UPDATEQuery.hpp"

#include <SparqlLexer/SparqlLexer.h>
#include <SparqlParser/SparqlParser.h>

#include "dice/sparql2tensor/parser/visitors/PrologueVisitor.hpp"
#include "dice/sparql2tensor/parser/visitors/UpdateQueryVisitor.hpp"
#include "dice/sparql2tensor/parser/exception/SPARQLErrorListener.hpp"

namespace dice::sparql2tensor {

	UPDATEQuery UPDATEQuery::parse(std::string const &sparql_update_str) {
		parser::exception::SPARQLErrorListener error_listener{};
		antlr4::ANTLRInputStream input(sparql_update_str);
		Dice::sparql_parser::base::SparqlLexer lexer(&input);
		antlr4::CommonTokenStream tokens(&lexer);
		Dice::sparql_parser::base::SparqlParser parser(&tokens);
		parser.removeErrorListeners();
		parser.addErrorListener(&error_listener);

		auto update_ctx = parser.updateCommand();

		UPDATEQuery update_query{};
		robin_hood::unordered_map<std::string, std::string> prefixes;
		parser::visitors::PrologueVisitor p_visitor{};
		for (auto prefix_ctx : update_ctx->prologue()) {
			auto cur_prefixes = p_visitor.visitPrologue(prefix_ctx).as<robin_hood::unordered_map<std::string, std::string>>();
			prefixes.insert(cur_prefixes.begin(), cur_prefixes.end());
		}

		update_query.prefixes_ = std::move(prefixes);

		parser::visitors::UpdateQueryVisitor update_query_visitor{&update_query};
		update_query_visitor.visitUpdateCommand(update_ctx);

		return update_query;
	}

}// namespace dice::sparql2tensor