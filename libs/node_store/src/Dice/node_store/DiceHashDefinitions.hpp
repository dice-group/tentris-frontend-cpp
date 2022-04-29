#ifndef TENTRIS_DICEHASHDEFINITIONS_HPP
#define TENTRIS_DICEHASHDEFINITIONS_HPP

#include <Dice/hash/DiceHash.hpp>

#include "Dice/node_store/MetallBNodeBackend.hpp"
#include "Dice/node_store/MetallIRIBackend.hpp"
#include "Dice/node_store/MetallLiteralBackend.hpp"
#include "Dice/node_store/MetallVariableBackend.hpp"


namespace Dice::hash {
	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::identifier::NodeID> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::identifier::NodeID const &x) noexcept {
			return Dice::hash::DiceHash<size_t, Policy>()(x.value());
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::view::LiteralBackendView> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::view::LiteralBackendView const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(std::make_tuple(
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.datatype_id.value()),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.lexical_form),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.language_tag)));
		}
	};
	template<typename Policy>
	struct dice_hash_overload<Policy, Dice::node_store::MetallLiteralBackend> {
		inline static std::size_t dice_hash(Dice::node_store::MetallLiteralBackend const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(std::make_tuple(
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.datatype_id().value()),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.lexical_form()),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.language_tag())));
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::view::BNodeBackendView> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::view::BNodeBackendView const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(x.identifier);
		}
	};
	template<typename Policy>
	struct dice_hash_overload<Policy, Dice::node_store::MetallBNodeBackend> {
		inline static std::size_t dice_hash(Dice::node_store::MetallBNodeBackend const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(x.identifier());
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::view::VariableBackendView> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::view::VariableBackendView const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(std::make_tuple(
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.is_anonymous),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.name)));
		}
	};
	template<typename Policy>
	struct dice_hash_overload<Policy, Dice::node_store::MetallVariableBackend> {
		inline static std::size_t dice_hash(Dice::node_store::MetallVariableBackend const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(std::make_tuple(
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.is_anonymous()),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.name())));
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::view::IRIBackendView> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::view::IRIBackendView const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(x.identifier);
		}
	};
	template<typename Policy>
	struct dice_hash_overload<Policy, Dice::node_store::MetallIRIBackend> {
		inline static std::size_t dice_hash(Dice::node_store::MetallIRIBackend const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(x.identifier());
		}
	};
}// namespace Dice::hash

namespace Dice::node_store {
	template<typename T>
	struct NodeBackendHash {
		size_t operator()(T const &node) const noexcept {
			return Dice::hash::DiceHashMartinus<T>()(node);
		}

		size_t operator()(typename metall_manager::allocator_type<T const>::pointer const &node_ptr) const noexcept {
			return Dice::hash::DiceHashMartinus<T const>()(*node_ptr);
		}

		size_t operator()(typename metall_manager::allocator_type<T>::pointer const &node_ptr) const noexcept {
			return Dice::hash::DiceHashMartinus<T>()(*node_ptr);
		}

		size_t operator()(rdf4cpp::rdf::storage::node::view::LiteralBackendView const &x) const noexcept {
			return Dice::hash::DiceHashMartinus<rdf4cpp::rdf::storage::node::view::LiteralBackendView>()(x);
		}
		size_t operator()(rdf4cpp::rdf::storage::node::view::BNodeBackendView const &x) const noexcept {
			return Dice::hash::DiceHashMartinus<rdf4cpp::rdf::storage::node::view::BNodeBackendView>()(x);
		}
		size_t operator()(rdf4cpp::rdf::storage::node::view::VariableBackendView const &x) const noexcept {
			return Dice::hash::DiceHashMartinus<rdf4cpp::rdf::storage::node::view::VariableBackendView>()(x);
		}
		size_t operator()(rdf4cpp::rdf::storage::node::view::IRIBackendView const &x) const noexcept {
			return Dice::hash::DiceHashMartinus<rdf4cpp::rdf::storage::node::view::IRIBackendView>()(x);
		}
	};
}// namespace Dice::node_store

#endif//TENTRIS_DICEHASHDEFINITIONS_HPP
