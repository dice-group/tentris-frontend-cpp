#ifndef TENTRIS_PERSISTENTNODESTORAGEBACKENDIMPL_HPP
#define TENTRIS_PERSISTENTNODESTORAGEBACKENDIMPL_HPP

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

#include "Dice/node_store/DiceHashDefinitions.hpp"
#include "Dice/node_store/MetallBNodeBackend.hpp"
#include "Dice/node_store/MetallIRIBackend.hpp"
#include "Dice/node_store/MetallLiteralBackend.hpp"
#include "Dice/node_store/MetallVariableBackend.hpp"


namespace Dice::node_store {

	struct NodeIDEqual {
		using NodeID = rdf4cpp::rdf::storage::node::identifier::NodeID;

		bool operator()(NodeID const &lhs, NodeID const &rhs) const noexcept {
			return lhs.value() == rhs.value();
		}
	};

	struct MetallPtrEqual {
		using is_transparent = void;
		using LiteralBackendView = rdf4cpp::rdf::storage::node::view::LiteralBackendView;
		using BNodeBackendView = rdf4cpp::rdf::storage::node::view::BNodeBackendView;
		using IRIBackendView = rdf4cpp::rdf::storage::node::view::IRIBackendView;
		using VariableBackendView = rdf4cpp::rdf::storage::node::view::VariableBackendView;

		bool operator()(MetallBNodeBackend::pointer_t const &self, MetallBNodeBackend::pointer_t const &other) const noexcept {
			if (self == other) return true;
			else if (not self or not other)
				return false;
			else
				return (*self) == (*other);
		}

		bool operator()(MetallIRIBackend::pointer_t const &self, MetallIRIBackend::pointer_t const &other) const noexcept {
			if (self == other) return true;
			else if (not self or not other)
				return false;
			else
				return (*self) == (*other);
		}

		bool operator()(MetallLiteralBackend::pointer_t const &self, MetallLiteralBackend::pointer_t const &other) const noexcept {
			if (self == other) return true;
			else if (not self or not other)
				return false;
			else
				return (*self) == (*other);
		}

		bool operator()(MetallVariableBackend::pointer_t const &self, MetallVariableBackend::pointer_t const &other) const noexcept {
			if (self == other) return true;
			else if (not self or not other)
				return false;
			else
				return (*self) == (*other);
		}


		bool operator()(const Dice::node_store::MetallBNodeBackend::pointer_t &self, const BNodeBackendView &other) const noexcept {
			if (self)
				return BNodeBackendView{.identifier = self->identifier()} == other;
			else
				return false;
		}

		bool operator()(const BNodeBackendView &self, const Dice::node_store::MetallBNodeBackend::pointer_t &other) const noexcept {
			return operator()(other, self);
		}

		bool operator()(const Dice::node_store::MetallIRIBackend::pointer_t &self, const IRIBackendView &other) const noexcept {
			if (self)
				return IRIBackendView{.identifier = self->identifier()} == other;
			else
				return false;
		}

		bool operator()(const IRIBackendView &self, const Dice::node_store::MetallIRIBackend::pointer_t &other) const noexcept {
			return operator()(other, self);
		}

		bool operator()(const Dice::node_store::MetallLiteralBackend::pointer_t &self, const LiteralBackendView &other) const noexcept {
			if (self)
				return LiteralBackendView{.datatype_id = self->datatype_id(), .lexical_form = self->lexical_form(), .language_tag = self->language_tag()} == other;
			else
				return false;
		}

		bool operator()(const LiteralBackendView &self, const Dice::node_store::MetallLiteralBackend::pointer_t &other) const noexcept {
			return operator()(other, self);
		}

		bool operator()(const Dice::node_store::MetallVariableBackend::pointer_t &self, const VariableBackendView &other) const noexcept {
			if (self)
				return VariableBackendView{.name = self->name(), .is_anonymous = self->is_anonymous()} == other;
			else
				return false;
		}

		bool operator()(const VariableBackendView &self, const Dice::node_store::MetallVariableBackend::pointer_t &other) const noexcept {
			return operator()(other, self);
		}
	};

	class PersistentNodeStorageBackendImpl {
		using RDFNodeType = rdf4cpp::rdf::storage::node::identifier::RDFNodeType;
		using NodeID = rdf4cpp::rdf::storage::node::identifier::NodeID;
		using LiteralType = rdf4cpp::rdf::storage::node::identifier::LiteralType;
		using LiteralID = rdf4cpp::rdf::storage::node::identifier::LiteralID;
		using LiteralBackendView = rdf4cpp::rdf::storage::node::view::LiteralBackendView;
		using BNodeBackendView = rdf4cpp::rdf::storage::node::view::BNodeBackendView;
		using IRIBackendView = rdf4cpp::rdf::storage::node::view::IRIBackendView;
		using VariableBackendView = rdf4cpp::rdf::storage::node::view::VariableBackendView;

	public:
		template<typename T>
		using pointer = typename metall_manager::allocator_type<T>::pointer;

	private:
		using NodeIDHash = Dice::hash::DiceHashMartinus<size_t>;

		template<typename T>
		using Index = tsl::sparse_map<size_t,
									  pointer<T>,
									  NodeIDHash,
									  std::equal_to<>,
									  metall_manager::allocator_type<std::pair<size_t, pointer<T>>>>;
		/*metall::container::unordered_map<NodeID,
									  pointer<T>,
									  NodeIDHash,
									  std::equal_to<>>;*/
		template<typename T>
		using ReverseIndex = tsl::sparse_map<pointer<T>,
											 size_t,
											 NodeBackendHash<T>,
											 MetallPtrEqual,
											 metall_manager::allocator_type<std::pair<pointer<T>, size_t>>>;
		/*metall::container::unordered_map<pointer<T>,
											 NodeID,
											 NodeBackendHash<T>,
											 std::equal_to<>>;*/

		metall_manager::allocator_type<std::byte> allocator;

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
		NodeID next_bnode_id = NodeID::min_bnode_id;
		NodeID next_iri_id = NodeID::min_iri_id;
		NodeID next_variable_id = NodeID::min_variable_id;


	public:
		explicit PersistentNodeStorageBackendImpl(metall_manager::allocator_type<std::byte> const&allocator);


		[[nodiscard]] NodeID find_or_make_id(BNodeBackendView const &) noexcept;
		[[nodiscard]] NodeID find_or_make_id(IRIBackendView const &) noexcept;
		[[nodiscard]] NodeID find_or_make_id(LiteralBackendView const &) noexcept;
		[[nodiscard]] NodeID find_or_make_id(VariableBackendView const &) noexcept;

		[[nodiscard]] NodeID find_id(BNodeBackendView const &) const noexcept;
		[[nodiscard]] NodeID find_id(IRIBackendView const &) const noexcept;
		[[nodiscard]] NodeID find_id(LiteralBackendView const &) const noexcept;
		[[nodiscard]] NodeID find_id(VariableBackendView const &) const noexcept;

		[[nodiscard]] IRIBackendView find_iri_backend_view(NodeID id) const;
		[[nodiscard]] LiteralBackendView find_literal_backend_view(NodeID id) const;
		[[nodiscard]] BNodeBackendView find_bnode_backend_view(NodeID id) const;
		[[nodiscard]] VariableBackendView find_variable_backend_view(NodeID id) const;
	};

}// namespace Dice::node_store

#endif//TENTRIS_PERSISTENTNODESTORAGEBACKENDIMPL_HPP
