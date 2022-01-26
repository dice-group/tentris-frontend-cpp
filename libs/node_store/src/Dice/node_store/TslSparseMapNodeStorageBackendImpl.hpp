#ifndef TENTRIS_TSLSPARSEMAPNODESTORAGEBACKENDIMPL_HPP
#define TENTRIS_TSLSPARSEMAPNODESTORAGEBACKENDIMPL_HPP

#include <boost/container/vector.hpp>
#include <shared_mutex>
#include <tsl/boost_offset_pointer.h>
#include <tsl/sparse_map.h>

#include <Dice/hash/DiceHash.hpp>
#include <rdf4cpp/rdf/storage/node/INodeStorageBackend.hpp>

#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>
//#include <metall/container/unordered_map.hpp>

#include "Dice/node_store/MetallBNodeBackend.hpp"
#include "Dice/node_store/MetallIRIBackend.hpp"
#include "Dice/node_store/MetallLiteralBackend.hpp"
#include "Dice/node_store/MetallVariableBackend.hpp"

namespace Dice::hash {
	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::identifier::NodeIDValue> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::identifier::NodeIDValue const &x) noexcept {
			return Dice::hash::DiceHash<size_t, Policy>()(x.value);
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::handle::LiteralBackendView> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::handle::LiteralBackendView const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(std::make_tuple(
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.datatype_id.raw()),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.lexical_form),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.language_tag)));
		}
	};
	template<typename Policy>
	struct dice_hash_overload<Policy, Dice::node_storage::MetallLiteralBackend> {
		inline static std::size_t dice_hash(Dice::node_storage::MetallLiteralBackend const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(std::make_tuple(
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.datatype_id().raw()),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.lexical_form()),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.language_tag())));
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::handle::BNodeBackendView> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::handle::BNodeBackendView const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(x.identifier);
		}
	};
	template<typename Policy>
	struct dice_hash_overload<Policy, Dice::node_storage::MetallBNodeBackend> {
		inline static std::size_t dice_hash(Dice::node_storage::MetallBNodeBackend const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(x.identifier());
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::handle::VariableBackendView> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::handle::VariableBackendView const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(std::make_tuple(
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.is_anonymous),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.name)));
		}
	};
	template<typename Policy>
	struct dice_hash_overload<Policy, Dice::node_storage::MetallVariableBackend> {
		inline static std::size_t dice_hash(Dice::node_storage::MetallVariableBackend const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(std::make_tuple(
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.is_anonymous()),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.name())));
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::handle::IRIBackendView> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::handle::IRIBackendView const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(x.identifier);
		}
	};
	template<typename Policy>
	struct dice_hash_overload<Policy, Dice::node_storage::MetallIRIBackend> {
		inline static std::size_t dice_hash(Dice::node_storage::MetallIRIBackend const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(x.identifier());
		}
	};
}// namespace Dice::hash

namespace rdf4cpp::rdf::storage::node::handle {
	//	template<typename T>
	//	using offset_ptr = typename metall::manager::allocator_type<T>::pointer;

	inline auto operator<=>(Dice::node_storage::MetallLiteralBackend::pointer_t const &self, LiteralBackendView const &other) noexcept {
		return LiteralBackendView{.datatype_id = self->datatype_id(), .lexical_form = self->lexical_form(), .language_tag = self->language_tag()} <=> other;
	}
	inline auto operator==(Dice::node_storage::MetallLiteralBackend::pointer_t const &self, LiteralBackendView const &other) noexcept {
		return LiteralBackendView{.datatype_id = self->datatype_id(), .lexical_form = self->lexical_form(), .language_tag = self->language_tag()} == other;
	}

	inline auto operator<=>(Dice::node_storage::MetallIRIBackend::pointer_t const &self, IRIBackendView const &other) noexcept {
		return IRIBackendView{.identifier = self->identifier()} <=> other;
	}
	inline auto operator==(Dice::node_storage::MetallIRIBackend::pointer_t const &self, IRIBackendView const &other) noexcept {
		return IRIBackendView{.identifier = self->identifier()} == other;
	}

	inline auto operator<=>(Dice::node_storage::MetallBNodeBackend::pointer_t const &self, BNodeBackendView const &other) noexcept {
		return BNodeBackendView{.identifier = self->identifier()} <=> other;
	}
	inline auto operator==(Dice::node_storage::MetallBNodeBackend::pointer_t const &self, BNodeBackendView const &other) noexcept {
		return BNodeBackendView{.identifier = self->identifier()} == other;
	}

	inline auto operator<=>(Dice::node_storage::MetallVariableBackend::pointer_t const &self, VariableBackendView const &other) noexcept {
		return VariableBackendView{.name = self->name(), .is_anonymous = self->is_anonymous()} <=> other;
	}
	inline auto operator==(Dice::node_storage::MetallVariableBackend::pointer_t const &self, VariableBackendView const &other) noexcept {
		return VariableBackendView{.name = self->name(), .is_anonymous = self->is_anonymous()} == other;
	}
}// namespace rdf4cpp::rdf::storage::node::handle

namespace Dice::node_storage {


	class TslSparseMapNodeStorageBackendImpl {
		using RDFNodeType = rdf4cpp::rdf::storage::node::identifier::RDFNodeType;
		using NodeIDValue = rdf4cpp::rdf::storage::node::identifier::NodeIDValue;
		using LiteralType = rdf4cpp::rdf::storage::node::identifier::LiteralType;
		using LiteralID = rdf4cpp::rdf::storage::node::identifier::LiteralID;
		using NodeID = rdf4cpp::rdf::storage::node::identifier::NodeID;
		using LiteralBackendView = rdf4cpp::rdf::storage::node::handle::LiteralBackendView;
		using BNodeBackendView = rdf4cpp::rdf::storage::node::handle::BNodeBackendView;
		using IRIBackendView = rdf4cpp::rdf::storage::node::handle::IRIBackendView;
		using VariableBackendView = rdf4cpp::rdf::storage::node::handle::VariableBackendView;

		template<typename T>
		struct NodeBackendHash {
			size_t operator()(T const &node) const noexcept {
				return Dice::hash::DiceHashxxh3<T>()(node);
			}

			size_t operator()(typename metall::manager::allocator_type<T const>::pointer const &node_ptr) const noexcept {
				return Dice::hash::DiceHashxxh3<T const>()(*node_ptr);
			}

			size_t operator()(typename metall::manager::allocator_type<T>::pointer const &node_ptr) const noexcept {
				return Dice::hash::DiceHashxxh3<T>()(*node_ptr);
			}

			size_t operator()(rdf4cpp::rdf::storage::node::handle::LiteralBackendView const &x) const noexcept {
				return Dice::hash::DiceHashxxh3<rdf4cpp::rdf::storage::node::handle::LiteralBackendView>()(x);
			}
			size_t operator()(rdf4cpp::rdf::storage::node::handle::BNodeBackendView const &x) const noexcept {
				return Dice::hash::DiceHashxxh3<rdf4cpp::rdf::storage::node::handle::BNodeBackendView>()(x);
			}
			size_t operator()(rdf4cpp::rdf::storage::node::handle::VariableBackendView const &x) const noexcept {
				return Dice::hash::DiceHashxxh3<rdf4cpp::rdf::storage::node::handle::VariableBackendView>()(x);
			}
			size_t operator()(rdf4cpp::rdf::storage::node::handle::IRIBackendView const &x) const noexcept {
				return Dice::hash::DiceHashxxh3<rdf4cpp::rdf::storage::node::handle::IRIBackendView>()(x);
			}
		};


	public:
		template<typename T>
		using pointer = typename metall::manager::allocator_type<T>::pointer;

	private:
		using NodeIDValueHash = Dice::hash::DiceHashMartinus<NodeIDValue>;

		template<typename T>
		using Index = tsl::sparse_map<NodeIDValue,
									  pointer<T>,
									  NodeIDValueHash,
									  std::equal_to<>,
									  metall::manager::allocator_type<std::pair<NodeIDValue, pointer<T>>>>;
		/*metall::container::unordered_map<NodeIDValue,
									  pointer<T>,
									  NodeIDValueHash,
									  std::equal_to<>>;*/
		template<typename T>
		using ReverseIndex = tsl::sparse_map<pointer<T>,
											 NodeIDValue,
											 NodeBackendHash<T>,
											 std::equal_to<>,
											 metall::manager::allocator_type<std::pair<pointer<T>, NodeIDValue>>>;
		/*metall::container::unordered_map<pointer<T>,
											 NodeIDValue,
											 NodeBackendHash<T>,
											 std::equal_to<>>;*/

		metall::manager::allocator_type<std::byte> allocator;

		constexpr static rdf4cpp::rdf::storage::node::identifier::NodeStorageID manager_id = rdf4cpp::rdf::storage::node::identifier::NodeStorageID{0};

		mutable std::shared_mutex literal_mutex_;
		Index<MetallLiteralBackend> literal_storage;
		ReverseIndex<MetallLiteralBackend> literal_storage_reverse;
		mutable std::shared_mutex bnode_mutex_;
		Index<MetallBNodeBackend> bnode_storage;
		ReverseIndex<MetallBNodeBackend> bnode_storage_reverse;
		mutable std::shared_mutex iri_mutex_;
		Index<MetallIRIBackend> iri_storage;
		ReverseIndex<MetallIRIBackend> iri_storage_reverse;
		mutable std::shared_mutex variable_mutex_;
		Index<MetallVariableBackend> variable_storage;
		ReverseIndex<MetallVariableBackend> variable_storage_reverse;

		LiteralID next_literal_id = NodeID::min_literal_id;
		NodeIDValue next_bnode_id = NodeID::min_bnode_id;
		NodeIDValue next_iri_id = NodeID::min_iri_id;
		NodeIDValue next_variable_id = NodeID::min_variable_id;


	public:
		explicit TslSparseMapNodeStorageBackendImpl(const metall::manager::allocator_type<std::byte> &allocator);

		[[nodiscard]] NodeID get_string_literal_id(std::string_view lexical_form) {
			return lookup_or_insert_literal(
						   LiteralBackendView{.datatype_id = NodeID{manager_id, RDFNodeType::IRI, NodeID::xsd_string_iri.first},
											  .lexical_form = lexical_form,
											  .language_tag = {}})
					.second;
		}

		[[nodiscard]] NodeID get_typed_literal_id(std::string_view lexical_form, std::string_view datatype) {
			return lookup_or_insert_literal(
						   LiteralBackendView{.datatype_id = lookup_or_insert_iri(IRIBackendView{.identifier = datatype}).second,
											  .lexical_form = lexical_form,
											  .language_tag = {}})
					.second;
		}

		[[nodiscard]] NodeID get_typed_literal_id(std::string_view lexical_form, const NodeID &datatype_id) {
			return lookup_or_insert_literal(
						   LiteralBackendView{.datatype_id = datatype_id,
											  .lexical_form = lexical_form,
											  .language_tag = {}})
					.second;
		}

		[[nodiscard]] NodeID get_lang_literal_id(std::string_view lexical_form, std::string_view lang) {
			return lookup_or_insert_literal(
						   LiteralBackendView{.datatype_id = NodeID{manager_id, RDFNodeType::IRI, NodeID::rdf_langstring_iri.first},
											  .lexical_form = lexical_form,
											  .language_tag = lang})
					.second;
		}

		[[nodiscard]] NodeID get_iri_id(std::string_view iri) {
			return lookup_or_insert_iri(IRIBackendView{.identifier = iri}).second;
		}

		[[nodiscard]] NodeID get_variable_id(std::string_view identifier, bool anonymous) {
			return lookup_or_insert_variable(VariableBackendView{.name = identifier, .is_anonymous = anonymous}).second;
		}

		[[nodiscard]] NodeID get_bnode_id(std::string_view identifier) {
			return lookup_or_insert_bnode(BNodeBackendView{.identifier = identifier}).second;
		}

		[[nodiscard]] IRIBackendView get_iri_handle(NodeIDValue id) const {
			return IRIBackendView(*iri_storage.at(id));
		}

		[[nodiscard]] LiteralBackendView get_literal_handle(NodeIDValue id) const {
			return LiteralBackendView(*literal_storage.at(id));
		}

		[[nodiscard]] BNodeBackendView get_bnode_handle(NodeIDValue id) const {
			return BNodeBackendView(*bnode_storage.at(id));
		}

		[[nodiscard]] VariableBackendView get_variable_handle(NodeIDValue id) const {
			return VariableBackendView(*variable_storage.at(id));
		}

	private:
		std::pair<MetallLiteralBackend::pointer_t, NodeID> lookup_or_insert_literal(LiteralBackendView literal);

		std::pair<MetallIRIBackend::pointer_t, NodeID> lookup_or_insert_iri(IRIBackendView iri);

		std::pair<MetallBNodeBackend::pointer_t, NodeID> lookup_or_insert_bnode(BNodeBackendView bnode);

		std::pair<MetallVariableBackend::pointer_t, NodeID> lookup_or_insert_variable(VariableBackendView variable);
	};

}// namespace Dice::node_storage

#endif//TENTRIS_TSLSPARSEMAPNODESTORAGEBACKENDIMPL_HPP
