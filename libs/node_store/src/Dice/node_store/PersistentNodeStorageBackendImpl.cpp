#include "PersistentNodeStorageBackendImpl.hpp"

#include <memory>
#include <mutex>
#include <utility>

namespace Dice::node_store {

	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::get_string_literal_id(std::string_view lexical_form) {
		return lookup_or_insert_literal(
					   LiteralBackendView{.datatype_id = NodeID{manager_id, RDFNodeType::IRI, NodeID::xsd_string_iri.first},
										  .lexical_form = lexical_form,
										  .language_tag = {}})
				.second;
	}
	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::get_typed_literal_id(std::string_view lexical_form, std::string_view datatype) {
		return lookup_or_insert_literal(
					   LiteralBackendView{.datatype_id = lookup_or_insert_iri(IRIBackendView{.identifier = datatype}).second,
										  .lexical_form = lexical_form,
										  .language_tag = {}})
				.second;
	}
	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::get_typed_literal_id(std::string_view lexical_form, const PersistentNodeStorageBackendImpl::NodeID &datatype_id) {
		return lookup_or_insert_literal(
					   LiteralBackendView{.datatype_id = datatype_id,
										  .lexical_form = lexical_form,
										  .language_tag = {}})
				.second;
	}
	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::get_lang_literal_id(std::string_view lexical_form, std::string_view lang) {
		return lookup_or_insert_literal(
					   LiteralBackendView{.datatype_id = NodeID{manager_id, RDFNodeType::IRI, NodeID::rdf_langstring_iri.first},
										  .lexical_form = lexical_form,
										  .language_tag = lang})
				.second;
	}
	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::get_iri_id(std::string_view iri) {
		return lookup_or_insert_iri(IRIBackendView{.identifier = iri}).second;
	}
	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::get_variable_id(std::string_view identifier, bool anonymous) {
		return lookup_or_insert_variable(VariableBackendView{.name = identifier, .is_anonymous = anonymous}).second;
	}
	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::get_bnode_id(std::string_view identifier) {
		return lookup_or_insert_bnode(BNodeBackendView{.identifier = identifier}).second;
	}
	PersistentNodeStorageBackendImpl::IRIBackendView PersistentNodeStorageBackendImpl::get_iri_handle(PersistentNodeStorageBackendImpl::NodeIDValue id) const {
		return IRIBackendView(*iri_storage.at(id));
	}
	PersistentNodeStorageBackendImpl::LiteralBackendView PersistentNodeStorageBackendImpl::get_literal_handle(PersistentNodeStorageBackendImpl::NodeIDValue id) const {
		return LiteralBackendView(*literal_storage.at(id));
	}
	PersistentNodeStorageBackendImpl::BNodeBackendView PersistentNodeStorageBackendImpl::get_bnode_handle(PersistentNodeStorageBackendImpl::NodeIDValue id) const {
		return BNodeBackendView(*bnode_storage.at(id));
	}
	PersistentNodeStorageBackendImpl::VariableBackendView PersistentNodeStorageBackendImpl::get_variable_handle(PersistentNodeStorageBackendImpl::NodeIDValue id) const {
		return VariableBackendView(*variable_storage.at(id));
	}

	std::pair<MetallLiteralBackend::pointer_t, rdf4cpp::rdf::storage::node::identifier::NodeID> PersistentNodeStorageBackendImpl::lookup_or_insert_literal(rdf4cpp::rdf::storage::node::handle::LiteralBackendView literal) {

		std::shared_lock<std::shared_mutex> shared_lock{literal_mutex_};
		auto found = literal_storage_reverse.find(literal /*, literal_storage_reverse.hash_function(), literal_storage_reverse.key_eq()*/);
		NodeID id;
		if (found == literal_storage_reverse.end()) {
			shared_lock.unlock();
			std::unique_lock<std::shared_mutex> unique_lock{literal_mutex_};
			// update found (might have changed in the meantime)
			found = literal_storage_reverse.find(literal /*, literal_storage_reverse.hash_function(), literal_storage_reverse.key_eq()*/);
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

	std::pair<MetallIRIBackend::pointer_t, rdf4cpp::rdf::storage::node::identifier::NodeID> PersistentNodeStorageBackendImpl::lookup_or_insert_iri(rdf4cpp::rdf::storage::node::handle::IRIBackendView iri) {

		std::shared_lock<std::shared_mutex> shared_lock{iri_mutex_};

		auto found = iri_storage_reverse.find(iri /*, iri_storage_reverse.hash_function(), iri_storage_reverse.key_eq()*/);
		NodeID id;
		if (found == iri_storage_reverse.end()) {
			shared_lock.unlock();
			std::unique_lock<std::shared_mutex> unique_lock{iri_mutex_};
			// update found (might have changed in the meantime)
			found = iri_storage_reverse.find(iri /*, iri_storage_reverse.hash_function(), iri_storage_reverse.key_eq()*/);
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
	std::pair<MetallBNodeBackend::pointer_t, rdf4cpp::rdf::storage::node::identifier::NodeID> PersistentNodeStorageBackendImpl::lookup_or_insert_bnode(rdf4cpp::rdf::storage::node::handle::BNodeBackendView bnode) {
		std::shared_lock<std::shared_mutex> shared_lock{bnode_mutex_};
		auto found = bnode_storage_reverse.find(bnode /*, bnode_storage_reverse.hash_function(), bnode_storage_reverse.key_eq()*/);
		NodeID id;

		if (found == bnode_storage_reverse.end()) {
			shared_lock.unlock();
			std::unique_lock<std::shared_mutex> unique_lock{bnode_mutex_};
			// update found (might have changed in the meantime)
			found = bnode_storage_reverse.find(bnode /*, bnode_storage_reverse.hash_function(), bnode_storage_reverse.key_eq()*/);
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
	std::pair<MetallVariableBackend::pointer_t, rdf4cpp::rdf::storage::node::identifier::NodeID> PersistentNodeStorageBackendImpl::lookup_or_insert_variable(rdf4cpp::rdf::storage::node::handle::VariableBackendView variable) {
		std::shared_lock<std::shared_mutex> shared_lock{variable_mutex_};
		auto found = variable_storage_reverse.find(variable /*, variable_storage_reverse.hash_function(), variable_storage_reverse.key_eq()*/);
		NodeID id;
		if (found == variable_storage_reverse.end()) {
			shared_lock.unlock();
			std::unique_lock<std::shared_mutex> unique_lock{variable_mutex_};
			// update found (might have changed in the meantime)
			found = variable_storage_reverse.find(variable /*, variable_storage_reverse.hash_function(), variable_storage_reverse.key_eq()*/);
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
	PersistentNodeStorageBackendImpl::PersistentNodeStorageBackendImpl(const metall::manager::allocator_type<std::byte> &allocator)
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

}// namespace Dice::node_store