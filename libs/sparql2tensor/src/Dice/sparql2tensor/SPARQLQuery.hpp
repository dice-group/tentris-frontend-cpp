#ifndef DICE_SPARQL_PARSEDSPARQL_HPP
#define DICE_SPARQL_PARSEDSPARQL_HPP

#include <rdf4cpp/rdf.hpp>

#include <Dice/hypertrie.hpp>
#include <Dice/rdf_tensor/HypertrieTrait.hpp>
#include <Dice/rdf_tensor/Query.hpp>
#include <Dice/rdf_tensor/RDFNodeHashes.hpp>
#include <Dice/rdf_tensor/Query.hpp>

#include "expressions/expressions.hpp"

#include <robin_hood.h>

namespace Dice::sparql2tensor {

	using VariableHash = Dice::hash::DiceHashxxh3<rdf4cpp::rdf::query::Variable>;

	struct SPARQLQuery {
		// graph representation of the query's graph patterns
		Dice::query::OperandDependencyGraph odg_;
		// a mapping of variables to unique (char) ids
		robin_hood::unordered_map<rdf4cpp::rdf::query::Variable, char, VariableHash> var_to_id_;
		// the order of the variables that appear in the select clause or other parts of the query that require their values (e.g., expressions)
		robin_hood::unordered_map<rdf4cpp::rdf::query::Variable, size_t, VariableHash> tracked_variables_;
		// the projected variables
		std::vector<rdf4cpp::rdf::query::Variable> projected_variables_;
		// the expressions of the select clause
		expressions::ExpressionList solution_;
		// maps variables to expressions
		robin_hood::unordered_map<rdf4cpp::rdf::query::Variable, std::unique_ptr<expressions::Expression>, VariableHash> aliases_;
		// the grouping keys of the query
		std::vector<std::unique_ptr<expressions::Expression>> grouping_keys_;
		// the triple patterns of the query
		std::vector<rdf4cpp::rdf::query::TriplePattern> triple_patterns_;
		// the prefixes of the query
		robin_hood::unordered_map<std::string, std::string> prefixes_;
		// a flag capturing whether the query is distinct
		bool distinct_ = false;
		// a flag capturing whether the query is an ask query
		bool ask_ = false;
		// a flag capturing whether the query contains aggregate expressions
		bool contains_aggregates_ = false;
		SPARQLQuery() = default;
		// creates a SPARQL query object by parsing the provided query string
		explicit SPARQLQuery(std::string const &sparql_query_str) : SPARQLQuery(SPARQLQuery::parse(sparql_query_str)) {}
		// parses a select or ask query string
		static SPARQLQuery parse(std::string const &sparql_query_str);
		// generates the slice keys from the triple patterns
		[[nodiscard]] std::vector<rdf_tensor::SliceKey> get_slice_keys() const;
	};

}// namespace Dice::sparql2tensor

#endif//DICE_SPARQL_PARSEDSPARQL_HPP