#ifndef DICE_SPARQL_PARSEDSPARQL_HPP
#define DICE_SPARQL_PARSEDSPARQL_HPP

#include <rdf4cpp/rdf.hpp>

#include <Dice/einsum/Subscript.hpp>
#include <Dice/hypertrie.hpp>

#include "Dice/sparql2tensor/BoolHypertrie.hpp"

#include <robin_hood.h>

namespace Dice::sparql2tensor {

	struct SPARQLQuery {
		std::vector<rdf4cpp::rdf::query::Variable> projected_variables_;

		robin_hood::unordered_map<rdf4cpp::rdf::query::Variable, char, Dice::hash::DiceHashxxh3<rdf4cpp::rdf::query::Variable>> var_to_id_;

		std::vector<rdf4cpp::rdf::query::TriplePattern> triple_patterns_;

		robin_hood::unordered_map<std::string, std::string> prefixes_;

		bool distinct_ = false;

		bool project_all_variables_ = false;

		SPARQLQuery() = default;

		static SPARQLQuery parse(std::string const &sparql_query_str);

		SPARQLQuery(std::string const &sparql_query_str) : SPARQLQuery(SPARQLQuery::parse(sparql_query_str)) {}

		bool is_distinct() const noexcept;

		std::vector<Dice::hypertrie::SliceKey<tr>> get_slice_keys() const;

		std::shared_ptr<einsum::Subscript> get_subscript() const;
	};

}// namespace Dice::sparql2tensor

#endif//DICE_SPARQL_PARSEDSPARQL_HPP