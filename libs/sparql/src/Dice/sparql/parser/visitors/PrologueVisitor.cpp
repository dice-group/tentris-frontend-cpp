#include "PrologueVisitor.hpp"

namespace Dice::sparql::parser::visitors {

	antlrcpp::Any PrologueVisitor::visitPrologue(SparqlParser::PrologueContext *ctx)  {
		prefixes.clear();
		for (auto pref_ctx : ctx->prefixDecl())
			visitPrefixDecl(pref_ctx);
		for ([[maybe_unused]] auto base_ctx : ctx->baseDecl())
			throw std::runtime_error("Base Declarations not supported yet.");
		return prefixes;
	}

	antlrcpp::Any PrologueVisitor::visitBaseDecl([[maybe_unused]] SparqlParser::BaseDeclContext *ctx) {
		return nullptr;
	}

	antlrcpp::Any PrologueVisitor::visitPrefixDecl(SparqlParser::PrefixDeclContext *ctx) {
		std::string prefix{};
		if (ctx->PNAME_NS())
			prefix = ctx->PNAME_NS()->getText();
		auto ns = ctx->IRIREF()->getText();
		prefixes[prefix.substr(0, prefix.size()-1)] = ns.substr(1, ns.size()-2);
		return nullptr;
	}


}// namespace Dice::sparql::parser::visitors