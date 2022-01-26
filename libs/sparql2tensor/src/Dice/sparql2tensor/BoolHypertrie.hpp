#ifndef TENTRIS_BOOLHYPERTRIE_HPP
#define TENTRIS_BOOLHYPERTRIE_HPP

#include <boost/container/vector.hpp>

#include <Dice/einsum.hpp>
#include <Dice/hash/DiceHash.hpp>
#include <Dice/hypertrie.hpp>
#include <rdf4cpp/rdf.hpp>
#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>
//#include <metall/container/unordered_map.hpp>
//#include <metall/container/unordered_set.hpp>

namespace Dice::hash {
	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::Node> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::Node const &x) noexcept {
			return std::hash<rdf4cpp::rdf::Node>()(x);
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::query::Variable> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::query::Variable const &x) noexcept {
			return std::hash<rdf4cpp::rdf::query::Variable>()(x);
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::Literal> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::Literal const &x) noexcept {
			return std::hash<rdf4cpp::rdf::Literal>()(x);
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::IRI> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::IRI const &x) noexcept {
			return std::hash<rdf4cpp::rdf::IRI>()(x);
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::BlankNode> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::BlankNode const &x) noexcept {
			return std::hash<rdf4cpp::rdf::BlankNode>()(x);
		}
	};
}// namespace Dice::hash

namespace Dice::sparql2tensor {
	using key_part_type = rdf4cpp::rdf::Node;

	// boost::container::unordered_{map,set} -> slow but works for sure
	/*template<typename key_part_type, typename value_type, typename Allocator>
	using map_type = metall::container::unordered_map<key_part_type, value_type, Dice::hash::DiceHashxxh3<key_part_type>,
													  std::equal_to<key_part_type>,
													  typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<const key_part_type, value_type>>>;
	template<typename key_part_type, typename Allocator>
	using set_type = metall::container::unordered_set<key_part_type, Dice::hash::DiceHashxxh3<key_part_type>,
													  std::equal_to<key_part_type>,
													  typename std::allocator_traits<Allocator>::template rebind_alloc<key_part_type>>;*/

	using tr = Dice::hypertrie::Hypertrie_trait<key_part_type,
												bool,
												metall::manager::allocator_type<std::byte>,
												Dice::hypertrie::internal::container::tsl_sparse_map,
												Dice::hypertrie::internal::container::tsl_sparse_set,
												63>;
	using HypertrieContext = Dice::hypertrie::HypertrieContext<tr>;
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
}// namespace Dice::sparql2tensor

#endif//TENTRIS_BOOLHYPERTRIE_HPP
