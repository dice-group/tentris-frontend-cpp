#ifndef TENTRIS_BOOLHYPERTRIE_HPP
#define TENTRIS_BOOLHYPERTRIE_HPP

#include "tentris/store/RDF/TermStore.hpp"
#include <Dice/hypertrie.hpp>
#include <Dice/query.hpp>

namespace tentris::tensor {
	using key_part_type = store::rdf::TermStore::ptr_type;

	using tr = Dice::hypertrie::Hypertrie_trait<key_part_type,
												bool,
												std::allocator<std::byte>,
												Dice::hypertrie::internal::container::tsl_sparse_map,
												Dice::hypertrie::internal::container::tsl_sparse_set,
												63>;
	using BoolHypertrie = Dice::hypertrie::Hypertrie<tr>;
	using const_BoolHypertrie = Dice::hypertrie::const_Hypertrie<tr>;
	using HypertrieBulkInserter = Dice::hypertrie::BulkInserter<tr>;
	using SliceKey = Dice::hypertrie::SliceKey<tr>;
	using Key = Dice::hypertrie::Key<tr>;
	using NonZeroEntry = Dice::hypertrie::NonZeroEntry<tr>;

	template<bool Distinct>
	using RESULT_TYPE = std::conditional_t<Distinct, bool, std::size_t>;
	template<typename result_type>
	using Solution = Dice::query::Entry<result_type, tr>;
	using Query = Dice::query::Query<tr>;
	using DISTINCT_t = bool;
	using COUNTED_t = std::size_t;

}// namespace tentris::tensor

#endif//TENTRIS_BOOLHYPERTRIE_HPP
