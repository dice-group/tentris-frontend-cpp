#ifndef TENTRIS_STORE_TRIPLESTORE
#define TENTRIS_STORE_TRIPLESTORE
#include "Dice/sparql2tensor/BoolHypertrie.hpp"
#include "Dice/sparql2tensor/SPARQLQuery.hpp"
#include <iostream>
#include <optional>
#include <string>
#include <vector>


namespace Dice::triple_store {

	class TripleStore {

		sparql2tensor::HypertrieContext context_;
		sparql2tensor::BoolHypertrie hypertrie_;

	public:
		TripleStore();

		[[nodiscard]] sparql2tensor::BoolHypertrie const &get_hypertrie() const;

		void load_ttl(
				const std::string &file_path,
				uint32_t bulk_size = 1'000'000,
				sparql2tensor::HypertrieBulkInserter::BulkInserted_callback const &call_back = [](size_t, size_t, size_t) -> void {});

		void add_statement(const rdf4cpp::rdf::Statement &statement);

		std::generator<sparql2tensor::EinsumEntry<sparql2tensor::COUNTED_t> const &> query(
				sparql2tensor::SPARQLQuery query,
				Dice::einsum::internal::Context::time_point endtime = Dice::einsum::internal::Context::time_point::max());

		bool contains(const rdf4cpp::rdf::Statement &statement);

		[[nodiscard]] size_t size() const;
	};
};    // namespace Dice::endpoint
#endif//TENTRIS_STORE_TRIPLESTORE
