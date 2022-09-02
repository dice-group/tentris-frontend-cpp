#ifndef DICE_SPARQL_SPARQLQUERY_HPP
#define DICE_SPARQL_SPARQLQUERY_HPP

#include <rdf4cpp/rdf.hpp>

#include <dice/hypertrie.hpp>
#include <dice/rdf-tensor/HypertrieTrait.hpp>
#include <dice/rdf-tensor/Query.hpp>
#include <dice/rdf-tensor/RDFNodeHashes.hpp>
#include <dice/triple-store/TripleStore.hpp>

#include "expressions/expressions.hpp"

#include <robin_hood.h>

namespace dice::sparql2tensor {

	using VariableHash = dice::hash::DiceHashxxh3<rdf4cpp::rdf::query::Variable>;

	/**
	 * @brief Represents a SPARQL query. Encapsulates an rdf_tensor::Query object.
	 */
	class SPARQLQuery {
	private:
		// the raw query object
		rdf_tensor::Query raw_query_;
		// a mapping of variables to unique (char) ids
		robin_hood::unordered_map<rdf4cpp::rdf::query::Variable, char, VariableHash> var_to_id_;
		// the projected variables
		std::vector<rdf4cpp::rdf::query::Variable> projected_variables_;
		// the triple patterns of the query
		std::vector<rdf4cpp::rdf::query::TriplePattern> triple_patterns_;
		// a flag capturing whether the query is an ask query
		bool ask_ = false;
		// the next available var_id
		char next_var_id = 'a';

	public:
		SPARQLQuery() = default;
		// returns a copy of raw_query_. returns a copy to keep a "clean" copy of the current object (*this) in the cache
		[[nodiscard]] rdf_tensor::Query raw_query() const;
		// returns a reference to the projected variables of the SPARQL query
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> const &projected_variables() const;
		// returns whether the SPARQL query is an ASK query
		[[nodiscard]] bool ask() const;
		// sets the value of ask_ to true; for ASK queries
		void set_ask();
		// assigns an id to the provided variable
		void register_variable(rdf4cpp::rdf::query::Variable var);
		// appends a variable to projected_variables_
		void add_projected_variable(rdf4cpp::rdf::query::Variable var);

		/* wrappers for rdf_tensor::Query methods */

		[[nodiscard]] rdf_tensor::operand_desc add_triple_pattern(rdf4cpp::rdf::query::TriplePattern const &tp,
																  dice::triple_store::TripleStore const &triple_store);

		[[nodiscard]] rdf_tensor::operand_desc add_filter_expr(std::unique_ptr<expressions::SPARQLExpression> expression,
															   dice::triple_store::TripleStore const &triple_store);

		void add_dependency(rdf_tensor::operand_desc operand_1, rdf_tensor::operand_desc operand_2, bool bidirectional = true);

		void add_connection(rdf_tensor::operand_desc operand_1, rdf_tensor::operand_desc operand_2, bool bidirectional = true);

		void track_variable(rdf4cpp::rdf::query::Variable variable);

		[[nodiscard]] size_t tracked_variable_position(rdf4cpp::rdf::query::Variable variable);

		[[nodiscard]] char variable_id(rdf4cpp::rdf::query::Variable variable);

		void add_solution_binding(std::unique_ptr<expressions::SPARQLExpression> expression);

		void add_grouping_expression(std::unique_ptr<expressions::SPARQLExpression> expression);

		void set_distinct();

		void set_aggregates();

		[[nodiscard]] bool contains_aggregates() const;
	};

}// namespace dice::sparql2tensor

#endif//DICE_SPARQL_SPARQLQUERY_HPP