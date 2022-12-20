#ifndef DICE_SPARQL_UPDATEQUERY_HPP
#define DICE_SPARQL_UPDATEQUERY_HPP

#include <dice/rdf-tensor/HypertrieTrait.hpp>


namespace dice::sparql2tensor {

	struct UPDATEDATAQueryData {
		bool is_delete; // is this query DELETE DATA? (otherwise is INSERT DATA)
		std::vector<rdf_tensor::NonZeroEntry> entries;
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
		rdf_tensor::parser::IStreamQuadIterator::prefix_storage_type prefixes;
		std::variant<UPDATEDATAQueryData, UPDATEQUERYQueryData, LOADQueryData, CLEARQueryData> query_data;

		UPDATEQuery() = default;
		static UPDATEQuery parse(std::string_view sparql_update_str);
	};
}

#endif//DICE_SPARQL_UPDATEQUERY_HPP
