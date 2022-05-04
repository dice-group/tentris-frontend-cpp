#include "PersistentNodeStorageBackendImpl.hpp"

#include <memory>
#include <mutex>
#include <utility>

namespace Dice::node_store {

	PersistentNodeStorageBackendImpl::PersistentNodeStorageBackendImpl(metall_manager::allocator_type<std::byte> const &allocator)
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
			metall_manager::allocator_type<MetallIRIBackend> alloc = allocator;
			auto mem = alloc.allocate(1);
			alloc.construct(mem, iri, alloc);
			auto [iter, inserted_successfully] = iri_storage_reverse.emplace(mem, id.value());
			assert(inserted_successfully);
			iri_storage.insert({id.value(), iter->first.get()});
		}
	}
	template<class Backend_t, bool create_if_not_present, class View_t, class Storage_t, class ReverseStorage_t, class Allocator = void*, class NextIDFromView_func = void *>
	inline rdf4cpp::rdf::storage::node::identifier::NodeID lookup_or_insert_impl(View_t const &view, std::shared_mutex &mutex, Storage_t &storage,
																				 ReverseStorage_t &reverse_storage, Allocator *allocator = nullptr,
																				 NextIDFromView_func next_id_func = nullptr) noexcept {
		using NodeID = rdf4cpp::rdf::storage::node::identifier::NodeID;
		std::shared_lock<std::shared_mutex> shared_lock{mutex};
		auto found = reverse_storage.find(view);
		if (found == reverse_storage.end()) {
			if constexpr (create_if_not_present) {
				shared_lock.unlock();
				std::unique_lock<std::shared_mutex> unique_lock{mutex};
				// update found (might have changed in the meantime)
				found = reverse_storage.find(view);
				if (found == reverse_storage.end()) {
					rdf4cpp::rdf::storage::node::identifier::NodeID id = next_id_func(view);
					metall_manager::allocator_type<Backend_t> alloc = *allocator;
					auto mem = alloc.allocate(1);
					assert(mem);
					alloc.construct(mem, view, alloc);
					auto [found2, inserted_successfully] = reverse_storage.emplace(mem, id.value());
					auto &element = *found2->first;
					auto x = View_t(element);
					assert(inserted_successfully);
					storage.insert({id.value(), found2->first.get()});
					return id;
				} else {
					unique_lock.unlock();
					return NodeID{found->second};
				}
			} else {
				return NodeID{};
			}
		} else {
			shared_lock.unlock();
			return NodeID{found->second};
		}
	}


	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::find_or_make_id(const PersistentNodeStorageBackendImpl::BNodeBackendView &view) noexcept {
		return lookup_or_insert_impl<MetallBNodeBackend, true>(
				view, bnode_mutex_, bnode_storage, bnode_storage_reverse, &allocator,
				[this](BNodeBackendView const &) {
					// TODO: actually use BnodeType (therefore, we will need bnode_view)
					return next_bnode_id++;
				});
	}
	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::find_or_make_id(const PersistentNodeStorageBackendImpl::IRIBackendView &view) noexcept {
		return lookup_or_insert_impl<MetallIRIBackend, true>(
				view, iri_mutex_, iri_storage, iri_storage_reverse,&allocator,
				[this](IRIBackendView const &) {
					return next_iri_id++;
				});
	}
	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::find_or_make_id(const PersistentNodeStorageBackendImpl::LiteralBackendView &view) noexcept {
		return lookup_or_insert_impl<MetallLiteralBackend, true>(
				view, literal_mutex_, literal_storage, literal_storage_reverse,&allocator,
				[this](LiteralBackendView const &) {
					// TODO: actually use LiteralType (therefore, we will need literal_view)
					return NodeID{next_literal_id++, LiteralType::OTHER};
				});
	}
	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::find_or_make_id(const PersistentNodeStorageBackendImpl::VariableBackendView &view) noexcept {
		return lookup_or_insert_impl<MetallVariableBackend, true>(
				view, variable_mutex_, variable_storage, variable_storage_reverse,&allocator,
				[this](VariableBackendView const &) {
					return next_variable_id++;
				});
	}
	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::find_id(const PersistentNodeStorageBackendImpl::BNodeBackendView &view) const noexcept {
		return lookup_or_insert_impl<MetallBNodeBackend, false>(
				view, bnode_mutex_, bnode_storage, bnode_storage_reverse);
	}
	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::find_id(const PersistentNodeStorageBackendImpl::IRIBackendView &view) const noexcept {
		return lookup_or_insert_impl<MetallIRIBackend, false>(
				view, iri_mutex_, iri_storage, iri_storage_reverse);
	}
	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::find_id(const PersistentNodeStorageBackendImpl::LiteralBackendView &view) const noexcept {
		return lookup_or_insert_impl<MetallLiteralBackend, false>(
				view, literal_mutex_, literal_storage, literal_storage_reverse);
	}
	PersistentNodeStorageBackendImpl::NodeID PersistentNodeStorageBackendImpl::find_id(const PersistentNodeStorageBackendImpl::VariableBackendView &view) const noexcept {
		return lookup_or_insert_impl<MetallVariableBackend, false>(
				view, variable_mutex_, variable_storage, variable_storage_reverse);
	}
	PersistentNodeStorageBackendImpl::IRIBackendView PersistentNodeStorageBackendImpl::find_iri_backend_view(PersistentNodeStorageBackendImpl::NodeID id) const {
		std::shared_lock<std::shared_mutex> shared_lock{iri_mutex_};
		return IRIBackendView(*iri_storage.at(id.value()));
	}
	PersistentNodeStorageBackendImpl::LiteralBackendView PersistentNodeStorageBackendImpl::find_literal_backend_view(PersistentNodeStorageBackendImpl::NodeID id) const {
		std::shared_lock<std::shared_mutex> shared_lock{literal_mutex_};
		return LiteralBackendView(*literal_storage.at(id.value()));
	}
	PersistentNodeStorageBackendImpl::BNodeBackendView PersistentNodeStorageBackendImpl::find_bnode_backend_view(PersistentNodeStorageBackendImpl::NodeID id) const {
		std::shared_lock<std::shared_mutex> shared_lock{bnode_mutex_};
		return BNodeBackendView(*bnode_storage.at(id.value()));
	}
	PersistentNodeStorageBackendImpl::VariableBackendView PersistentNodeStorageBackendImpl::find_variable_backend_view(PersistentNodeStorageBackendImpl::NodeID id) const {
		std::shared_lock<std::shared_mutex> shared_lock{variable_mutex_};
		return VariableBackendView(*variable_storage.at(id.value()));
	}

}// namespace Dice::node_store