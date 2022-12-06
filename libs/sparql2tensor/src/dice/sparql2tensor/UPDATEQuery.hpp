#ifndef DICE_SPARQL_UPDATEQUERY_HPP
#define DICE_SPARQL_UPDATEQUERY_HPP

#include <dice/rdf-tensor/HypertrieTrait.hpp>


namespace dice::sparql2tensor {

	struct UPDATEDATAQueryData {
		bool is_delete; // is this query DELETE DATA? (otherwise is INSERT DATA)
		std::string raw_entry_data; // raw, unparsed triple data to avoid 1 unnecessary copy for every entry
	};

	struct UPDATEQUERYQueryData {
		// to be determined
	};

	struct LOADQueryData {
		rdf_tensor::IRI rdf_doc_path;
	};

	struct CLEARQueryData {
		// no data, as there is only a single graph
	};

	struct UPDATEQuery {
		robin_hood::unordered_map<std::string, std::string> prefixes;
		std::variant<UPDATEDATAQueryData, UPDATEQUERYQueryData, LOADQueryData, CLEARQueryData> query_data;

		UPDATEQuery() = default;
		static UPDATEQuery parse(std::string const &sparql_update_str);
	};
}

#endif//DICE_SPARQL_UPDATEQUERY_HPP
