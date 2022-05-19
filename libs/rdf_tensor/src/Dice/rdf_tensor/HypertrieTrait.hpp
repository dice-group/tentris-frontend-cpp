#ifndef TENTRIS_HYPERTRIETRAIT_HPP
#define TENTRIS_HYPERTRIETRAIT_HPP

#include "Dice/rdf_tensor/NodeWrapper.hpp"

#include <Dice/einsum/Subscript.hpp>
#include <Dice/hypertrie.hpp>
#include <tsl/sparse_map.h>
#include <tsl/sparse_set.h>

namespace Dice::rdf_tensor {
	using key_part_type = NodeWrapper;
	template<typename Key, typename T, typename Allocator>
	using map_type = tsl::sparse_map<Key,
									 T,
									 Dice::hash::DiceHashMartinus<Key>,
									 std::equal_to<Key>,
									 typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<Key, T>>,
									 tsl::sh::power_of_two_growth_policy<2>,
									 tsl::sh::exception_safety::basic,
									 tsl::sh::sparsity::high>;

	template<typename Key, typename Allocator>
	using set_type = tsl::sparse_set<
			Key,
			Dice::hash::DiceHashMartinus<Key>,
			std::equal_to<Key>,
			typename std::allocator_traits<Allocator>::template rebind_alloc<Key>,
			tsl::sh::power_of_two_growth_policy<2>,
			tsl::sh::exception_safety::basic,
			tsl::sh::sparsity::high>;

	using htt_t = Dice::hypertrie::Hypertrie_trait<key_part_type,
												   bool,
												   map_type,
												   set_type,
												   63>;

	using SliceKey = Dice::hypertrie::SliceKey<htt_t>;
	using Key = Dice::hypertrie::Key<htt_t>;
	using NonZeroEntry = Dice::hypertrie::NonZeroEntry<htt_t>;

	using Subscript = Dice::einsum::Subscript;
}// namespace Dice::rdf_tensor
#endif//TENTRIS_HYPERTRIETRAIT_HPP
