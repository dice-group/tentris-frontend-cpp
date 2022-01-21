#include "TslSparseMapNodeStorageBackend.hpp"

#include <memory>
#include <mutex>
#include <utility>

namespace Dice::node_storage {


	std::pair<rdf4cpp::rdf::storage::node::LiteralBackend *, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackend::get_string_literal(std::string_view lexical_form) {
		return lookup_or_insert_literal(LiteralBackend{lexical_form, NodeID{manager_id, RDFNodeType::IRI, NodeID::xsd_string_iri.first}});
	}
	std::pair<rdf4cpp::rdf::storage::node::LiteralBackend *, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackend::get_typed_literal(std::string_view lexical_form, std::string_view datatype) {
		return lookup_or_insert_literal(LiteralBackend{lexical_form, lookup_or_insert_iri(IRIBackend{datatype}).second});
	}
	std::pair<rdf4cpp::rdf::storage::node::LiteralBackend *, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackend::get_typed_literal(std::string_view lexical_form, const NodeID &datatype_id) {
		return lookup_or_insert_literal(LiteralBackend{lexical_form, datatype_id});
	}
	std::pair<rdf4cpp::rdf::storage::node::LiteralBackend *, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackend::get_lang_literal(std::string_view lexical_form, std::string_view lang) {
		return lookup_or_insert_literal(LiteralBackend{lexical_form, NodeID{manager_id, RDFNodeType::IRI, NodeID::rdf_langstring_iri.first}, lang});
	}
	std::pair<rdf4cpp::rdf::storage::node::IRIBackend *, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackend::get_iri(std::string_view iri) {
		return lookup_or_insert_iri(IRIBackend{iri});
	}
	std::pair<rdf4cpp::rdf::storage::node::VariableBackend *, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackend::get_variable(std::string_view identifier, bool anonymous) {
		return lookup_or_insert_variable(VariableBackend{identifier, anonymous});
	}
	std::pair<rdf4cpp::rdf::storage::node::BNodeBackend *, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackend::get_bnode(std::string_view identifier) {
		return lookup_or_insert_bnode(BNodeBackend{identifier});
	}
	rdf4cpp::rdf::storage::node::IRIBackend *TslSparseMapNodeStorageBackend::lookup_iri(NodeIDValue id) const {
		std::shared_lock<std::shared_mutex> shared_lock{iri_mutex_};
		return &(*iri_storage.at(id));
	}
	rdf4cpp::rdf::storage::node::LiteralBackend *TslSparseMapNodeStorageBackend::lookup_literal(NodeIDValue id) const {
		std::shared_lock<std::shared_mutex> shared_lock{literal_mutex_};
		return &(*literal_storage.at(id));
	}
	rdf4cpp::rdf::storage::node::BNodeBackend *TslSparseMapNodeStorageBackend::lookup_bnode(NodeIDValue id) const {
		std::shared_lock<std::shared_mutex> shared_lock{bnode_mutex_};

		return &(*bnode_storage.at(id));
	}
	rdf4cpp::rdf::storage::node::VariableBackend *TslSparseMapNodeStorageBackend::lookup_variable(NodeIDValue id) const {
		std::shared_lock<std::shared_mutex> shared_lock{variable_mutex_};
		return &(*variable_storage.at(id));
	}

	std::pair<rdf4cpp::rdf::storage::node::LiteralBackend *, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackend::lookup_or_insert_literal(LiteralBackend literal) {
		std::shared_lock<std::shared_mutex> shared_lock{literal_mutex_};
		auto found = literal_storage_reverse.find(literal);
		NodeID id;
		if (found == literal_storage_reverse.end()) {
			shared_lock.unlock();
			std::unique_lock<std::shared_mutex> unique_lock{literal_mutex_};
			// update found (might have changed in the meantime)
			found = literal_storage_reverse.find(literal);
			if (found == literal_storage_reverse.end()) {
				id = {manager_id, RDFNodeType::Literal, next_literal_id++, LiteralType::STRING};
				metall::manager::allocator_type<LiteralBackend> alloc = allocator;
				auto mem = alloc.allocate(1);
				alloc.construct(mem, std::move(literal));
				auto [found2, inserted_successfully] = literal_storage_reverse.emplace(mem, id.node_id());
				assert(inserted_successfully);
				found = found2;
				literal_storage.insert({id.node_id(), found->first.get()});
			} else {
				unique_lock.unlock();
				id = {manager_id, RDFNodeType::Literal, found->second};
			}
		} else {
			shared_lock.unlock();
			id = {manager_id, RDFNodeType::Literal, found->second};
		}

		return {found->first.get(), id};
	}

	std::pair<rdf4cpp::rdf::storage::node::IRIBackend *, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackend::lookup_or_insert_iri(IRIBackend iri) {
		std::shared_lock<std::shared_mutex> shared_lock{iri_mutex_};
		auto found = iri_storage_reverse.find(iri);
		NodeID id;
		if (found == iri_storage_reverse.end()) {
			shared_lock.unlock();
			std::unique_lock<std::shared_mutex> unique_lock{iri_mutex_};
			// update found (might have changed in the meantime)
			found = iri_storage_reverse.find(iri);
			if (found == iri_storage_reverse.end()) {
				id = {manager_id, RDFNodeType::IRI, next_iri_id++};
				metall::manager::allocator_type<IRIBackend> alloc = allocator;
				auto mem = alloc.allocate(1);
				alloc.construct(mem, std::move(iri));
				auto [found2, inserted_successfully] = iri_storage_reverse.emplace(mem, id.node_id());
				assert(inserted_successfully);
				found = found2;
				iri_storage.insert({id.node_id(), found->first.get()});
			} else {
				unique_lock.unlock();
				id = {manager_id, RDFNodeType::IRI, found->second};
			}
		} else {
			shared_lock.unlock();
			id = {manager_id, RDFNodeType::IRI, found->second};
		}
		return {found->first.get(), id};
	}
	std::pair<rdf4cpp::rdf::storage::node::BNodeBackend *, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackend::lookup_or_insert_bnode(BNodeBackend bnode) {
		std::shared_lock<std::shared_mutex> shared_lock{bnode_mutex_};
		auto found = bnode_storage_reverse.find(bnode);
		NodeID id;

		if (found == bnode_storage_reverse.end()) {
			shared_lock.unlock();
			std::unique_lock<std::shared_mutex> unique_lock{bnode_mutex_};
			// update found (might have changed in the meantime)
			found = bnode_storage_reverse.find(bnode);
			if (found == bnode_storage_reverse.end()) {
				id = {manager_id, RDFNodeType::BNode, next_bnode_id++};
				metall::manager::allocator_type<BNodeBackend> alloc = allocator;
				auto mem = alloc.allocate(1);
				alloc.construct(mem, std::move(bnode));
				auto [found2, inserted_successfully] = bnode_storage_reverse.emplace(mem, id.node_id());
				assert(inserted_successfully);
				found = found2;
				bnode_storage.insert({id.node_id(), found->first.get()});
			} else {
				unique_lock.unlock();
				id = {manager_id, RDFNodeType::BNode, found->second};
			}
		} else {
			shared_lock.unlock();
			id = {manager_id, RDFNodeType::BNode, found->second};
		}

		return {found->first.get(), id};
	}
	std::pair<rdf4cpp::rdf::storage::node::VariableBackend *, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackend::lookup_or_insert_variable(VariableBackend variable) {
		std::shared_lock<std::shared_mutex> shared_lock{variable_mutex_};
		auto found = variable_storage_reverse.find(variable);
		NodeID id;
		if (found == variable_storage_reverse.end()) {
			shared_lock.unlock();
			std::unique_lock<std::shared_mutex> unique_lock{variable_mutex_};
			// update found (might have changed in the meantime)
			found = variable_storage_reverse.find(variable);
			if (found == variable_storage_reverse.end()) {
				id = {manager_id, RDFNodeType::Variable, next_variable_id++};
				metall::manager::allocator_type<VariableBackend> alloc = allocator;
				auto mem = alloc.allocate(1);
				alloc.construct(mem, std::move(variable));
				auto [found2, inserted_successfully] = variable_storage_reverse.emplace(mem, id.node_id());
				assert(inserted_successfully);
				found = found2;
				variable_storage.insert({id.node_id(), found->first.get()});
			} else {
				unique_lock.unlock();
				id = {manager_id, RDFNodeType::Variable, found->second};
			}
		} else {
			shared_lock.unlock();
			id = {manager_id, RDFNodeType::Variable, found->second};
		}
		return {found->first.get(), id};
	}
	TslSparseMapNodeStorageBackend::TslSparseMapNodeStorageBackend(metall::manager::allocator_type<std::byte> allocator)
		: INodeStorageBackend(),
		  allocator(allocator),
		  literal_storage(allocator),
		  literal_storage_reverse(allocator),
		  bnode_storage(allocator),
		  bnode_storage_reverse(allocator),
		  iri_storage(allocator),
		  iri_storage_reverse(allocator),
		  variable_storage(allocator),
		  variable_storage_reverse(allocator) {
		// some iri's like xsd:string are there by default
		for (const auto &[id, iri] : NodeID::predefined_iris) {
			metall::manager::allocator_type<IRIBackend> alloc = allocator;
			auto mem = alloc.allocate(1);
			alloc.construct(mem, iri);
			auto [iter, inserted_successfully] = iri_storage_reverse.emplace(mem, id);
			assert(inserted_successfully);
			iri_storage.insert({id, iter->first.get()});
		}
	}
}// namespace Dice::node_storage