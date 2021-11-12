#ifndef TENTRIS_BOOLHYPERTRIE_HPP
#define TENTRIS_BOOLHYPERTRIE_HPP

#include "tentris/store/RDF/TermStore.hpp"
#include <Dice/einsum.hpp>
#include <Dice/hypertrie.hpp>

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

	template<typename result_type>
	using EinsumEntry = Dice::einsum::Entry<result_type, tr>;
	using DISTINCT_t = bool;
	using COUNTED_t = std::size_t;
	using Subscript = Dice::einsum::Subscript;
}// namespace tentris::tensor

#endif//TENTRIS_BOOLHYPERTRIE_HPP
