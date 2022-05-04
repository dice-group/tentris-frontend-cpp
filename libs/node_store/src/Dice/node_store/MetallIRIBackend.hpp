#ifndef RDF4CPP_METALLIRIBACKEND_HPP
#define RDF4CPP_METALLIRIBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/view/IRIBackendView.hpp>

#include <Dice/node_store/metall_manager.hpp>

#include <compare>
#include <memory>
#include <string>
#include <string_view>

namespace Dice::node_store {
	class MetallIRIBackend {
		metall_string iri;

	public:
		using pointer_t = metall_manager::allocator_type<MetallIRIBackend>::pointer;
		explicit MetallIRIBackend(std::string_view iri, metall_manager::allocator_type<std::byte> const &allocator) noexcept;
		explicit MetallIRIBackend(rdf4cpp::rdf::storage::node::view::IRIBackendView, metall_manager::allocator_type<std::byte> const &allocator) noexcept;
		std::partial_ordering operator<=>(const MetallIRIBackend &other) const noexcept;
		std::partial_ordering operator<=>(pointer_t const &other) const noexcept;
		bool operator==(const MetallIRIBackend &other) const noexcept = default;
		[[nodiscard]] std::string_view identifier() const noexcept;

		[[nodiscard]] std::string n_string() const noexcept;

		explicit operator rdf4cpp::rdf::storage::node::view::IRIBackendView() const noexcept;
	};

	std::partial_ordering operator<=>(MetallIRIBackend::pointer_t const &self, MetallIRIBackend::pointer_t const &other) noexcept;
}// namespace Dice::node_store

namespace rdf4cpp::rdf::storage::node::view {

	std::partial_ordering operator<=>(Dice::node_store::MetallIRIBackend::pointer_t const &self, IRIBackendView const &other) noexcept;
	bool operator==(Dice::node_store::MetallIRIBackend::pointer_t const &self, IRIBackendView const &other) noexcept;
}// namespace rdf4cpp::rdf::storage::node::view

#endif//RDF4CPP_METALLIRIBACKEND_HPP
