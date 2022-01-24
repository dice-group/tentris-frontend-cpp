#ifndef TENTRIS_TSLSPARSEMAPNODESTORAGEBACKEND_HPP
#define TENTRIS_TSLSPARSEMAPNODESTORAGEBACKEND_HPP

#include <shared_mutex>
#include <tsl/sparse_map.h>

#include <Dice/hash/DiceHash.hpp>
#include <rdf4cpp/rdf/storage/node/INodeStorageBackend.hpp>

#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>
#include <tsl/boost_offset_pointer.h>

#include "Dice/node_store/MetallBNodeBackend.hpp"
#include "Dice/node_store/MetallIRIBackend.hpp"
#include "Dice/node_store/MetallLiteralBackend.hpp"
#include "Dice/node_store/MetallVariableBackend.hpp"

namespace Dice::hash {
	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::NodeIDValue> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::NodeIDValue const &x) noexcept {
			return Dice::hash::DiceHash<size_t, Policy>()(x.value);
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::LiteralBackendHandle> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::LiteralBackendHandle const &x) noexcept {
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
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::BNodeBackendHandle> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::BNodeBackendHandle const &x) noexcept {
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
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::VariableBackendHandle> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::VariableBackendHandle const &x) noexcept {
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
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::IRIBackendHandle> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::IRIBackendHandle const &x) noexcept {
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

namespace rdf4cpp::rdf::storage::node {
	//	template<typename T>
	//	using offset_ptr = typename metall::manager::allocator_type<T>::pointer;

	inline auto operator<=>(Dice::node_storage::MetallLiteralBackend::pointer_t const &self, LiteralBackendHandle const &other) noexcept {
		return LiteralBackendHandle{.datatype_id = self->datatype_id(), .lexical_form = self->lexical_form(), .language_tag = self->language_tag()} <=> other;
	}
	inline auto operator==(Dice::node_storage::MetallLiteralBackend::pointer_t const &self, LiteralBackendHandle const &other) noexcept {
		return LiteralBackendHandle{.datatype_id = self->datatype_id(), .lexical_form = self->lexical_form(), .language_tag = self->language_tag()} == other;
	}

	inline auto operator<=>(Dice::node_storage::MetallIRIBackend::pointer_t const &self, IRIBackendHandle const &other) noexcept {
		return IRIBackendHandle{.identifier = self->identifier()} <=> other;
	}
	inline auto operator==(Dice::node_storage::MetallIRIBackend::pointer_t const &self, IRIBackendHandle const &other) noexcept {
		return IRIBackendHandle{.identifier = self->identifier()} == other;
	}

	inline auto operator<=>(Dice::node_storage::MetallBNodeBackend::pointer_t const &self, BNodeBackendHandle const &other) noexcept {
		return BNodeBackendHandle{.identifier = self->identifier()} <=> other;
	}
	inline auto operator==(Dice::node_storage::MetallBNodeBackend::pointer_t const &self, BNodeBackendHandle const &other) noexcept {
		return BNodeBackendHandle{.identifier = self->identifier()} == other;
	}

	inline auto operator<=>(Dice::node_storage::MetallVariableBackend::pointer_t const &self, VariableBackendHandle const &other) noexcept {
		return VariableBackendHandle{.name = self->name(), .is_anonymous = self->is_anonymous()} <=> other;
	}
	inline auto operator==(Dice::node_storage::MetallVariableBackend::pointer_t const &self, VariableBackendHandle const &other) noexcept {
		return VariableBackendHandle{.name = self->name(), .is_anonymous = self->is_anonymous()} == other;
	}
}// namespace rdf4cpp::rdf::storage::node

namespace Dice::node_storage {


	class TslSparseMapNodeStorageBackend : public rdf4cpp::rdf::storage::node::INodeStorageBackend {
		using RDFNodeType = rdf4cpp::rdf::storage::node::RDFNodeType;
		using NodeIDValue = rdf4cpp::rdf::storage::node::NodeIDValue;
		using LiteralType = rdf4cpp::rdf::storage::node::LiteralType;
		using LiteralID = rdf4cpp::rdf::storage::node::LiteralID;
		using NodeID = rdf4cpp::rdf::storage::node::NodeID;
		using LiteralBackendHandle = rdf4cpp::rdf::storage::node::LiteralBackendHandle;
		using BNodeBackendHandle = rdf4cpp::rdf::storage::node::BNodeBackendHandle;
		using IRIBackendHandle = rdf4cpp::rdf::storage::node::IRIBackendHandle;
		using VariableBackendHandle = rdf4cpp::rdf::storage::node::VariableBackendHandle;

		template<typename T>
		struct NodeBackendHash {
			size_t operator()(T const &node) const noexcept {
				return Dice::hash::DiceHashxxh3<T>()(node);
			}

			size_t operator()(typename metall::manager::allocator_type<T const>::pointer node_ptr) const noexcept {
				return Dice::hash::DiceHashxxh3<T const>()(*node_ptr);
			}

			size_t operator()(typename metall::manager::allocator_type<T>::pointer node_ptr) const noexcept {
				return Dice::hash::DiceHashxxh3<T>()(*node_ptr);
			}

			size_t operator()(rdf4cpp::rdf::storage::node::LiteralBackend const &x) const noexcept {
				return Dice::hash::DiceHashxxh3<rdf4cpp::rdf::storage::node::LiteralBackend>()(x);
			}
			size_t operator()(rdf4cpp::rdf::storage::node::LiteralBackendHandle const &x) const noexcept {
				return Dice::hash::DiceHashxxh3<rdf4cpp::rdf::storage::node::LiteralBackendHandle>()(x);
			}

			size_t operator()(rdf4cpp::rdf::storage::node::BNodeBackendHandle const &x) const noexcept {
				return Dice::hash::DiceHashxxh3<rdf4cpp::rdf::storage::node::BNodeBackendHandle>()(x);
			}
			size_t operator()(rdf4cpp::rdf::storage::node::BNodeBackend const &x) const noexcept {
				return Dice::hash::DiceHashxxh3<rdf4cpp::rdf::storage::node::BNodeBackend>()(x);
			}

			size_t operator()(rdf4cpp::rdf::storage::node::VariableBackendHandle const &x) const noexcept {
				return Dice::hash::DiceHashxxh3<rdf4cpp::rdf::storage::node::VariableBackendHandle>()(x);
			}
			size_t operator()(rdf4cpp::rdf::storage::node::VariableBackend const &x) const noexcept {
				return Dice::hash::DiceHashxxh3<rdf4cpp::rdf::storage::node::VariableBackend>()(x);
			}

			size_t operator()(rdf4cpp::rdf::storage::node::IRIBackendHandle const &x) const noexcept {
				return Dice::hash::DiceHashxxh3<rdf4cpp::rdf::storage::node::IRIBackendHandle>()(x);
			}
			size_t operator()(rdf4cpp::rdf::storage::node::IRIBackend const &x) const noexcept {
				return Dice::hash::DiceHashxxh3<rdf4cpp::rdf::storage::node::IRIBackend>()(x);
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
		template<typename T>
		using ReverseIndex = tsl::sparse_map<pointer<T>,
											 NodeIDValue,
											 NodeBackendHash<T>,
											 std::equal_to<>,
											 metall::manager::allocator_type<std::pair<pointer<T>, NodeIDValue>>>;

		metall::manager::allocator_type<std::byte> allocator;

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
		explicit TslSparseMapNodeStorageBackend(const metall::manager::allocator_type<std::byte>& allocator);

		void reset_use_count() {
			this->inc_use_count();
		}

		~TslSparseMapNodeStorageBackend() override = default;

		[[nodiscard]] NodeID get_string_literal_id(std::string_view lexical_form) override {
			return lookup_or_insert_literal(
						   LiteralBackendHandle{.datatype_id = NodeID{manager_id, RDFNodeType::IRI, NodeID::xsd_string_iri.first},
												.lexical_form = lexical_form,
												.language_tag = {}})
					.second;
		}

		[[nodiscard]] NodeID get_typed_literal_id(std::string_view lexical_form, std::string_view datatype) override {
			return lookup_or_insert_literal(
						   LiteralBackendHandle{.datatype_id = lookup_or_insert_iri(IRIBackendHandle{.identifier = datatype}).second,
												.lexical_form = lexical_form,
												.language_tag = {}})
					.second;
		}

		[[nodiscard]] NodeID get_typed_literal_id(std::string_view lexical_form, const NodeID &datatype_id) override {
			return lookup_or_insert_literal(
						   LiteralBackendHandle{.datatype_id = datatype_id,
												.lexical_form = lexical_form,
												.language_tag = {}})
					.second;
		}

		[[nodiscard]] NodeID get_lang_literal_id(std::string_view lexical_form, std::string_view lang) override {
			return lookup_or_insert_literal(
						   LiteralBackendHandle{.datatype_id = NodeID{manager_id, RDFNodeType::IRI, NodeID::rdf_langstring_iri.first},
												.lexical_form = lexical_form,
												.language_tag = lang})
					.second;
		}

		[[nodiscard]] NodeID get_iri_id(std::string_view iri) override {
			return lookup_or_insert_iri(IRIBackendHandle{.identifier = iri}).second;
		}

		[[nodiscard]] NodeID get_variable_id(std::string_view identifier, bool anonymous) override {
			return lookup_or_insert_variable(VariableBackendHandle{.name = identifier, .is_anonymous = anonymous}).second;
		}

		[[nodiscard]] NodeID get_bnode_id(std::string_view identifier) override {
			return lookup_or_insert_bnode(BNodeBackendHandle{.identifier = identifier}).second;
		}

		[[nodiscard]] IRIBackendHandle get_iri_handle(NodeIDValue id) const override {
			return IRIBackendHandle(*iri_storage.at(id));
		}

		[[nodiscard]] LiteralBackendHandle get_literal_handle(NodeIDValue id) const override {
			return LiteralBackendHandle(*literal_storage.at(id));
		}

		[[nodiscard]] BNodeBackendHandle get_bnode_handle(NodeIDValue id) const override {
			return BNodeBackendHandle(*bnode_storage.at(id));
		}

		[[nodiscard]] VariableBackendHandle get_variable_handle(NodeIDValue id) const override {
			return VariableBackendHandle(*variable_storage.at(id));
		}

	private:
		std::pair<MetallLiteralBackend::pointer_t, NodeID> lookup_or_insert_literal(LiteralBackendHandle literal);

		std::pair<MetallIRIBackend::pointer_t, NodeID> lookup_or_insert_iri(IRIBackendHandle iri);

		std::pair<MetallBNodeBackend::pointer_t, NodeID> lookup_or_insert_bnode(BNodeBackendHandle bnode);

		std::pair<MetallVariableBackend::pointer_t, NodeID> lookup_or_insert_variable(VariableBackendHandle variable);
	};

}// namespace Dice::node_storage

#endif//TENTRIS_TSLSPARSEMAPNODESTORAGEBACKEND_HPP
