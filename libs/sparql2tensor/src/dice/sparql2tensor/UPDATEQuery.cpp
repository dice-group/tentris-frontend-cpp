#include "UPDATEQuery.hpp"

#include <algorithm>
#include <type_traits>

#include <SparqlLexer/SparqlLexer.h>
#include <SparqlParser/SparqlParser.h>

#include "dice/sparql2tensor/parser/visitors/PrologueVisitor.hpp"
#include "dice/sparql2tensor/parser/visitors/UpdateQueryVisitor.hpp"
#include "dice/sparql2tensor/parser/exception/SPARQLErrorListener.hpp"

namespace dice::sparql2tensor {

	static bool is_alpha(char const ch) noexcept {
		return std::isalpha(ch);
	}

	static bool is_ws(char const ch) noexcept {
		return std::isspace(ch);
	}

	/**
	 * @brief reads a single word (determined by matcher) from the start of s
	 * @param s input string; will be modified to not include the word after extraction
	 * @param matcher determines the charset the word is made of
	 * @return the extracted word
	 * @todo when clang supports ranges properly: merge read_word and read_word_rev
	 */
	template<typename CharMatcher> requires std::is_nothrow_invocable_r_v<bool, CharMatcher, char>
	static std::string_view read_word(std::string_view &s, CharMatcher &&matcher) noexcept {
		auto const first_word_begin = std::find_if_not(s.begin(), s.end(), is_ws);
		auto const first_word_end = std::find_if_not(first_word_begin, s.end(), std::forward<CharMatcher>(matcher));

		auto word = s.substr(std::distance(s.begin(), first_word_begin), std::distance(first_word_begin, first_word_end));
		s.remove_prefix(std::distance(s.begin(), first_word_end));

		return word;
	}

	/**
	 * @brief reads a single word (determined by matcher) from the end of s
	 * @param s input string; will be modified to not include the word after extraction
	 * @param matcher determines the charset the word is made of
	 * @return the extracted word
	 */
	template<typename CharMatcher> requires std::is_nothrow_invocable_r_v<bool, CharMatcher, char>
	static std::string_view read_word_rev(std::string_view &s, CharMatcher &&matcher) noexcept {
		auto const first_word_rbegin = std::find_if_not(s.rbegin(), s.rend(), is_ws);
		auto const first_word_rend = std::find_if_not(first_word_rbegin, s.rend(), std::forward<CharMatcher>(matcher));

		auto word = s.substr(std::distance(first_word_rend, s.rend()), std::distance(first_word_rbegin, first_word_rend));
		s.remove_suffix(std::distance(s.rbegin(), first_word_rend));

		return word;
	}

	/**
	 * @brief extracts the prologue from an update query
	 * @param s the whole query, will be modified to not include the extracted prologue afterwards
	 * @return the extracted prologue
	 */
	static std::string_view read_prologue(std::string_view &s) noexcept {
		auto const query_body_begin = s.find_first_of('{');
		if (query_body_begin == std::string_view::npos) {
			// body begin not found, error will be handled by calling function
			return "";
		}

		auto const prologue_last_char = s.substr(0, query_body_begin).find_last_of('>');
		if (prologue_last_char == std::string_view::npos) {
			// no prologue found
			return "";
		}

		auto const prologue = s.substr(0, prologue_last_char + 1);
		s.remove_prefix(prologue_last_char + 1);

		return prologue;
	}

	UPDATEQuery UPDATEQuery::parse(std::string_view const sparql_update_str) {
		std::string_view rest_mut = sparql_update_str;
		auto const prologue = read_prologue(rest_mut);

		UPDATEQuery update_query;

		auto const first_word = read_word(rest_mut, is_alpha);
		auto const second_word = read_word(rest_mut, is_alpha);
		auto const third_word = read_word(rest_mut, [](char const ch) noexcept { return ch == '{'; });

		if (bool is_delete; ((is_delete = first_word == "DELETE") || first_word == "INSERT") && second_word == "DATA") {
			// fast path for DELETE DATA / INSERT DATA

			if (third_word != "{") {
				std::ostringstream err;
				err << "syntax error: expected '{' after " << first_word << " " << second_word;
				throw std::runtime_error{err.str()};
			}

			auto const last_word = read_word_rev(rest_mut, [](char const ch) noexcept { return ch == '}'; });

			if (last_word != "}") {
				throw std::runtime_error{"syntax error: expected '}' at end of query"};
			}

			using namespace rdf_tensor::parser;

			{ // prologue
				parser::exception::SPARQLErrorListener error_listener{};
				antlr4::ANTLRInputStream input{prologue};
				dice::sparql_parser::base::SparqlLexer lexer{&input};
				antlr4::CommonTokenStream tokens{&lexer};
				dice::sparql_parser::base::SparqlParser parser{&tokens};
				parser.removeErrorListeners();
				parser.addErrorListener(&error_listener);

				auto update_ctx = parser.updateCommand();

				{ // prologue
					parser::visitors::PrologueVisitor p_visitor{};
					for (auto prefix_ctx : update_ctx->prologue()) {
						auto cur_prefixes = std::any_cast<IStreamQuadIterator::prefix_storage_type>(p_visitor.visitPrologue(prefix_ctx));
						update_query.prefixes.insert(cur_prefixes.begin(), cur_prefixes.end());
					}
				}
			}

			std::vector<rdf_tensor::NonZeroEntry> entries;
			std::istringstream iss{std::string{rest_mut}};
			for (IStreamQuadIterator qit{iss, ParsingFlag::NoParsePrefix, update_query.prefixes}; qit != IStreamQuadIterator{}; ++qit) {
				if (qit->has_value()) {
					auto const &quad = **qit;
					entries.push_back(rdf_tensor::NonZeroEntry{{quad.subject(), quad.predicate(), quad.object()}});
				} else {
					std::ostringstream oss;
					oss << qit->error();
					throw std::runtime_error{oss.str()};
				}
			}

			update_query.query_data = UPDATEDATAQueryData{
					.is_delete = is_delete,
					.entries = std::move(entries)};
		} else {
			// parse whole input with antlr
			parser::exception::SPARQLErrorListener error_listener{};
			antlr4::ANTLRInputStream input{sparql_update_str};
			dice::sparql_parser::base::SparqlLexer lexer{&input};
			antlr4::CommonTokenStream tokens{&lexer};
			dice::sparql_parser::base::SparqlParser parser{&tokens};
			parser.removeErrorListeners();
			parser.addErrorListener(&error_listener);

			auto update_ctx = parser.updateCommand();

			{ // prologue
				parser::visitors::PrologueVisitor p_visitor{};
				for (auto prefix_ctx : update_ctx->prologue()) {
					auto cur_prefixes = std::any_cast<rdf_tensor::parser::IStreamQuadIterator::prefix_storage_type>(p_visitor.visitPrologue(prefix_ctx));
					update_query.prefixes.insert(cur_prefixes.begin(), cur_prefixes.end());
				}
			}

			parser::visitors::UpdateQueryVisitor update_query_visitor{update_query};
			update_query_visitor.visitUpdateCommand(update_ctx);
		}

		return update_query;
	}

}// namespace dice::sparql2tensor