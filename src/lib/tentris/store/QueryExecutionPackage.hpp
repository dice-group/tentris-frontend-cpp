#ifndef TENTRIS_QUERYEXECUTIONPACKAGE_HPP
#define TENTRIS_QUERYEXECUTIONPACKAGE_HPP

#include <any>
#include <exception>
#include <ostream>

#include <rdf4cpp/rdf.hpp>

#include "tentris/store/AtomicTripleStore.hpp"
#include "tentris/store/RDF/TermStore.hpp"
#include "tentris/tensor/BoolHypertrie.hpp"

#include <Dice/sparql/parser/ParsedSPARQL.hpp>
#include <Dice/sparql/parser/Parser.hpp>

namespace tentris::store {
	class TripleStore;
};

namespace tentris::store::cache {

	using namespace Dice::sparql::parser;
	/**
	 * A QueryExecutionPackage contains everything that is necessary to execute a given sparql query for a state of the
	 * RDF graph.
	 */
	struct QueryExecutionPackage {
		using const_BoolHypertrie = ::tentris::tensor::const_BoolHypertrie;
		using time_point_t = logging::time_point_t;
		using Variable = rdf4cpp::rdf::query::Variable;

	private:
		std::string sparql_string;

	public:
		/**
		 * Indicates if the QueryExecutionPackage represents an distinct query or not. If it is distinct use only
		 * the methods with distinct in their names. Otherwise use only the methods with regular in their names
		 */

		bool is_trivial_empty = false;

	private:
		ParsedSPARQL query;
		std::vector<const_BoolHypertrie> operands{};

	public:
		QueryExecutionPackage() = delete;

		/**
		 *
		 * @param sparql_string sparql query to be parsed
		 * @param trie current try holding the data
		 * @param termIndex term store attached to the trie
		 * @throw std::invalid_argument the sparql query was not parsable
		 */
		explicit QueryExecutionPackage(const std::string &sparql_string) : sparql_string{sparql_string} {
			using namespace logging;
			logDebug(fmt::format("Parsing query: {}", sparql_string));
			query = parse_query(sparql_string);

			auto &triple_store = AtomicTripleStore::getInstance();

			logDebug(fmt::format("Slicing TPs"));
			for ([[maybe_unused]] const auto &[op_pos, tp] : iter::enumerate(query.triple_patterns)) {
//				logDebug(fmt::format("Slice key {}: ⟨{}⟩", op_pos, fmt::join(tp, ", ")));
				std::variant<const_BoolHypertrie, bool> op = triple_store.resolveTriplePattern(tp);
				if (std::holds_alternative<bool>(op)) {
					is_trivial_empty = not std::get<bool>(op);
					logTrace(fmt::format("Operand {} is {}", op_pos, is_trivial_empty));
				} else {
					auto bht = std::get<const_BoolHypertrie>(op);
					if (not bht.empty()) {
						logTrace(fmt::format("Operand {} size {}", op_pos, bht.size()));
						operands.emplace_back(bht);
					} else {
						is_trivial_empty = true;
						operands.clear();
					}
				}
				if (is_trivial_empty) {
					logDebug(fmt::format("Query is trivially empty, i.e. the lastly sliced operand {} is emtpy.", op_pos));
					break;
				}
			}
		}


	public:
		[[nodiscard]] const std::vector<const_BoolHypertrie> &getOperands() const {
			return operands;
		}

		[[nodiscard]] ParsedSPARQL &getQuery() {
			return query;
		}

		friend struct ::fmt::formatter<QueryExecutionPackage>;
	};
}// namespace tentris::store::cache

template<>
struct fmt::formatter<tentris::store::cache::QueryExecutionPackage> {
	template<typename ParseContext>
	constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

	template<typename FormatContext>
	auto format(const tentris::store::cache::QueryExecutionPackage &p, FormatContext &ctx) {
		return format_to(ctx.begin(),
						 " SPARQL:     {}\n"
						 " is_distinct:      {}\n"
						 " is_trivial_empty: {}\n",
						 p.sparql_string,  p.query.distinct, p.is_trivial_empty);
	}
};

#endif// TENTRIS_QUERYEXECUTIONPACKAGE_HPP
