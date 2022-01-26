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

	struct NodeIDValueEqual {
		using NodeIDValue = rdf4cpp::rdf::storage::node::identifier::NodeIDValue;

		bool operator()(NodeIDValue const &lhs, NodeIDValue const &rhs) const noexcept {
			return lhs.value == rhs.value;
		}
	};

	class PersistentNodeStorageBackendImpl {
		using RDFNodeType = rdf4cpp::rdf::storage::node::identifier::RDFNodeType;
		using NodeIDValue = rdf4cpp::rdf::storage::node::identifier::NodeIDValue;
		using LiteralType = rdf4cpp::rdf::storage::node::identifier::LiteralType;
		using LiteralID = rdf4cpp::rdf::storage::node::identifier::LiteralID;
		using NodeID = rdf4cpp::rdf::storage::node::identifier::NodeID;
		using LiteralBackendView = rdf4cpp::rdf::storage::node::handle::LiteralBackendView;
		using BNodeBackendView = rdf4cpp::rdf::storage::node::handle::BNodeBackendView;
		using IRIBackendView = rdf4cpp::rdf::storage::node::handle::IRIBackendView;
		using VariableBackendView = rdf4cpp::rdf::storage::node::handle::VariableBackendView;

	public:
		template<typename T>
		using pointer = typename metall::manager::allocator_type<T>::pointer;

	private:
		using NodeIDValueHash = Dice::hash::DiceHashMartinus<NodeIDValue>;

		template<typename T>
		using Index = tsl::sparse_map<NodeIDValue,
									  pointer<T>,
									  NodeIDValueHash,
									  NodeIDValueEqual,
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
		explicit PersistentNodeStorageBackendImpl(const metall::manager::allocator_type<std::byte> &allocator);

		[[nodiscard]] NodeID get_string_literal_id(std::string_view lexical_form);

		[[nodiscard]] NodeID get_typed_literal_id(std::string_view lexical_form, std::string_view datatype);

		[[nodiscard]] NodeID get_typed_literal_id(std::string_view lexical_form, const NodeID &datatype_id);

		[[nodiscard]] NodeID get_lang_literal_id(std::string_view lexical_form, std::string_view lang);

		[[nodiscard]] NodeID get_iri_id(std::string_view iri);

		[[nodiscard]] NodeID get_variable_id(std::string_view identifier, bool anonymous);

		[[nodiscard]] NodeID get_bnode_id(std::string_view identifier);

		[[nodiscard]] IRIBackendView get_iri_handle(NodeIDValue id) const;

		[[nodiscard]] LiteralBackendView get_literal_handle(NodeIDValue id) const;

		[[nodiscard]] BNodeBackendView get_bnode_handle(NodeIDValue id) const;

		[[nodiscard]] VariableBackendView get_variable_handle(NodeIDValue id) const;

	private:
		std::pair<MetallLiteralBackend::pointer_t, NodeID> lookup_or_insert_literal(LiteralBackendView literal);

		std::pair<MetallIRIBackend::pointer_t, NodeID> lookup_or_insert_iri(IRIBackendView iri);

		std::pair<MetallBNodeBackend::pointer_t, NodeID> lookup_or_insert_bnode(BNodeBackendView bnode);

		std::pair<MetallVariableBackend::pointer_t, NodeID> lookup_or_insert_variable(VariableBackendView variable);
	};

}// namespace Dice::node_store

#endif//TENTRIS_PERSISTENTNODESTORAGEBACKENDIMPL_HPP
