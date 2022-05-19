#ifndef RDF4CPP_METALLIRIBACKEND_HPP
#define RDF4CPP_METALLIRIBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/view/IRIBackendView.hpp>

#include <Dice/node_store/metall_manager.hpp>

namespace Dice::node_store {
	class MetallIRIBackend {
		metall_string iri;
		size_t hash_;

	public:
		using View = rdf4cpp::rdf::storage::node::view::IRIBackendView;

		explicit MetallIRIBackend(std::string_view iri, metall_manager::allocator_type<std::byte> const &allocator) noexcept;
		explicit MetallIRIBackend(rdf4cpp::rdf::storage::node::view::IRIBackendView, metall_manager::allocator_type<std::byte> const &allocator) noexcept;
		[[nodiscard]] std::string_view identifier() const noexcept;

		[[nodiscard]] size_t hash() const noexcept { return hash_; }

		explicit operator rdf4cpp::rdf::storage::node::view::IRIBackendView() const noexcept;
	};

}// namespace Dice::node_store


#endif//RDF4CPP_METALLIRIBACKEND_HPP
