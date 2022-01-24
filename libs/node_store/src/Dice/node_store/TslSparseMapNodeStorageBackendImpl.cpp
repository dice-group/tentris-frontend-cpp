#include "TslSparseMapNodeStorageBackendImpl.hpp"

#include <memory>
#include <mutex>
#include <utility>

namespace Dice::node_storage {


	std::pair<MetallLiteralBackend::pointer_t, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackendImpl::lookup_or_insert_literal(rdf4cpp::rdf::storage::node::LiteralBackendHandle literal) {

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
				metall::manager::allocator_type<MetallLiteralBackend> alloc = allocator;
				auto mem = alloc.allocate(1);
				alloc.construct(mem, literal.lexical_form, literal.datatype_id, literal.language_tag, alloc);
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

	std::pair<MetallIRIBackend::pointer_t, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackendImpl::lookup_or_insert_iri(rdf4cpp::rdf::storage::node::IRIBackendHandle iri) {

		std::shared_lock<std::shared_mutex> shared_lock{iri_mutex_};
		std::cout << "lookup_or_insert_iri" << std::endl;
		std::cout << "size:" << iri_storage_reverse.size() << std::endl;
		std::cout << "iri:" << iri.identifier << std::endl;
		std::cout << "contains:" << iri_storage_reverse.contains(iri) << std::endl;

		auto found = iri_storage_reverse.find(iri);
		NodeID id;
		if (found == iri_storage_reverse.end()) {
			shared_lock.unlock();
			std::unique_lock<std::shared_mutex> unique_lock{iri_mutex_};
			// update found (might have changed in the meantime)
			found = iri_storage_reverse.find(iri);
			if (found == iri_storage_reverse.end()) {
				id = {manager_id, RDFNodeType::IRI, next_iri_id++};
				metall::manager::allocator_type<MetallIRIBackend> alloc = allocator;
				auto mem = alloc.allocate(1);
				alloc.construct(mem, iri.identifier, alloc);
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
	std::pair<MetallBNodeBackend::pointer_t, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackendImpl::lookup_or_insert_bnode(rdf4cpp::rdf::storage::node::BNodeBackendHandle bnode) {
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
				metall::manager::allocator_type<MetallBNodeBackend> alloc = allocator;
				auto mem = alloc.allocate(1);
				alloc.construct(mem, bnode.identifier, alloc);
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
	std::pair<MetallVariableBackend::pointer_t, rdf4cpp::rdf::storage::node::NodeID> TslSparseMapNodeStorageBackendImpl::lookup_or_insert_variable(rdf4cpp::rdf::storage::node::VariableBackendHandle variable) {
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
				metall::manager::allocator_type<MetallVariableBackend> alloc = allocator;
				auto mem = alloc.allocate(1);
				alloc.construct(mem, variable.name, variable.is_anonymous, alloc);
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
	TslSparseMapNodeStorageBackendImpl::TslSparseMapNodeStorageBackendImpl(const metall::manager::allocator_type<std::byte> &allocator)
		: allocator(allocator),
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
			metall::manager::allocator_type<MetallIRIBackend> alloc = allocator;
			auto mem = alloc.allocate(1);
			alloc.construct(mem, iri, alloc);
			auto [iter, inserted_successfully] = iri_storage_reverse.emplace(mem, id);
			assert(inserted_successfully);
			iri_storage.insert({id, iter->first.get()});
		}
	}
}// namespace Dice::node_storage