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

namespace Dice::hash {
	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::NodeIDValue> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::NodeIDValue const &x) noexcept {
			return Dice::hash::DiceHash<size_t, Policy>()(x.value);
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::LiteralBackend> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::LiteralBackend const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(std::make_tuple(
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.datatype_id().raw()),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.lexical_form()),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.language_tag())));
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::BNodeBackend> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::BNodeBackend const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(x.indentifier());
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::VariableBackend> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::VariableBackend const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(std::make_tuple(
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.is_anonymous()),
					Dice::hash::dice_hash_templates<Policy>::dice_hash(x.name())));
		}
	};

	template<typename Policy>
	struct dice_hash_overload<Policy, rdf4cpp::rdf::storage::node::IRIBackend> {
		inline static std::size_t dice_hash(rdf4cpp::rdf::storage::node::IRIBackend const &x) noexcept {
			return Dice::hash::dice_hash_templates<Policy>::dice_hash(x.identifier());
		}
	};
}// namespace Dice::hash

namespace rdf4cpp::rdf::storage::node {
	template<typename T>
	using offset_ptr = typename metall::manager::allocator_type<T>::pointer;

	inline std::strong_ordering operator<=>(LiteralBackend const &self, offset_ptr<LiteralBackend> const &other) noexcept{
		return self <=> *other;
	}

	inline std::strong_ordering operator<=>(IRIBackend const &self, offset_ptr<IRIBackend> const &other) noexcept{
		return self <=> *other;
	}

	inline std::strong_ordering operator<=>(BNodeBackend const &self, offset_ptr<BNodeBackend> const &other) noexcept{
		return self <=> *other;
	}

	inline std::strong_ordering operator<=>(VariableBackend const &self, offset_ptr<VariableBackend> const &other) noexcept{
		return self <=> *other;
	}
}

namespace Dice::node_storage {


	// TODO: stored Backend nodes must use boost::string
	// TODO: rdf4cpp should pass std::string_view to the outside (which should work with boost::string) as well
	class TslSparseMapNodeStorageBackend : public rdf4cpp::rdf::storage::node::INodeStorageBackend {
		using RDFNodeType = rdf4cpp::rdf::storage::node::RDFNodeType;
		using NodeIDValue = rdf4cpp::rdf::storage::node::NodeIDValue;
		using LiteralType = rdf4cpp::rdf::storage::node::LiteralType;
		using LiteralID = rdf4cpp::rdf::storage::node::LiteralID;
		using NodeID = rdf4cpp::rdf::storage::node::NodeID;
		using LiteralBackend = rdf4cpp::rdf::storage::node::LiteralBackend;
		using BNodeBackend = rdf4cpp::rdf::storage::node::BNodeBackend;
		using IRIBackend = rdf4cpp::rdf::storage::node::IRIBackend;
		using VariableBackend = rdf4cpp::rdf::storage::node::VariableBackend;

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
									  std::less<>,
									  metall::manager::allocator_type<std::pair<NodeIDValue, pointer<T>>>>;
		template<typename T>
		using ReverseIndex = tsl::sparse_map<pointer<T>,
											 NodeIDValue,
											 NodeBackendHash<T>,
											 std::less<>,
											 metall::manager::allocator_type<std::pair<pointer<T>, NodeIDValue>>>;

		metall::manager::allocator_type<std::byte> allocator;

		mutable std::shared_mutex literal_mutex_;
		Index<LiteralBackend> literal_storage;
		ReverseIndex<LiteralBackend> literal_storage_reverse;
		mutable std::shared_mutex bnode_mutex_;
		Index<BNodeBackend> bnode_storage;
		ReverseIndex<BNodeBackend> bnode_storage_reverse;
		mutable std::shared_mutex iri_mutex_;
		Index<IRIBackend> iri_storage;
		ReverseIndex<IRIBackend> iri_storage_reverse;
		mutable std::shared_mutex variable_mutex_;
		Index<VariableBackend> variable_storage;
		ReverseIndex<VariableBackend> variable_storage_reverse;

		LiteralID next_literal_id = NodeID::min_literal_id;
		NodeIDValue next_bnode_id = NodeID::min_bnode_id;
		NodeIDValue next_iri_id = NodeID::min_iri_id;
		NodeIDValue next_variable_id = NodeID::min_variable_id;



	public:
		TslSparseMapNodeStorageBackend(metall::manager::allocator_type<std::byte> allocator);

		~TslSparseMapNodeStorageBackend() override = default;

		[[nodiscard]] std::pair<LiteralBackend *, NodeID> get_string_literal(const std::string &lexical_form) override;

		[[nodiscard]] std::pair<LiteralBackend *, NodeID> get_typed_literal(const std::string &lexical_form, const std::string &datatype) override;

		[[nodiscard]] std::pair<LiteralBackend *, NodeID> get_typed_literal(const std::string &lexical_form, const NodeID &datatype_id) override;

		[[nodiscard]] std::pair<LiteralBackend *, NodeID> get_lang_literal(const std::string &lexical_form, const std::string &lang) override;

		[[nodiscard]] std::pair<IRIBackend *, NodeID> get_iri(const std::string &iri) override;

		[[nodiscard]] std::pair<VariableBackend *, NodeID> get_variable(const std::string &identifier, bool anonymous) override;

		[[nodiscard]] std::pair<BNodeBackend *, NodeID> get_bnode(const std::string &identifier) override;

		[[nodiscard]] IRIBackend *lookup_iri(NodeIDValue id) const override;

		[[nodiscard]] LiteralBackend *lookup_literal(NodeIDValue id) const override;

		[[nodiscard]] BNodeBackend *lookup_bnode(NodeIDValue id) const override;

		[[nodiscard]] VariableBackend *lookup_variable(NodeIDValue id) const override;

	private:
		std::pair<LiteralBackend *, NodeID> lookup_or_insert_literal(LiteralBackend literal);

		std::pair<IRIBackend *, NodeID> lookup_or_insert_iri(IRIBackend iri);

		std::pair<BNodeBackend *, NodeID> lookup_or_insert_bnode(BNodeBackend bnode);

		std::pair<VariableBackend *, NodeID> lookup_or_insert_variable(VariableBackend variable);
	};

}// namespace Dice::node_storage

#endif//TENTRIS_TSLSPARSEMAPNODESTORAGEBACKEND_HPP
