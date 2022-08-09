#ifndef DICE_SPARQL_UPDATEQUERY_HPP
#define DICE_SPARQL_UPDATEQUERY_HPP

#include <dice/rdf-tensor/HypertrieTrait.hpp>


namespace dice::sparql2tensor {

	struct UPDATEQuery {
		robin_hood::unordered_map<std::string, std::string> prefixes_;

		std::vector<rdf_tensor::NonZeroEntry> entries_for_removal;

		UPDATEQuery() = default;

		static UPDATEQuery parse(std::string const &sparql_update_str);

		UPDATEQuery(std::string const &sparql_update_str) : UPDATEQuery(UPDATEQuery::parse(sparql_update_str)) {}
	};

}

#endif//DICE_SPARQL_UPDATEQUERY_HPP
