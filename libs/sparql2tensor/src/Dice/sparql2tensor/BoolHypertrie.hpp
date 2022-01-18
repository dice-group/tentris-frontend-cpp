#ifndef TENTRIS_BOOLHYPERTRIE_HPP
#define TENTRIS_BOOLHYPERTRIE_HPP

#include <Dice/einsum.hpp>
#include <Dice/hash/DiceHash.hpp>
#include <Dice/hypertrie.hpp>
#include <rdf4cpp/rdf.hpp>

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

	using tr = Dice::hypertrie::Hypertrie_trait<key_part_type,
												bool,
												std::allocator<std::byte>,
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
