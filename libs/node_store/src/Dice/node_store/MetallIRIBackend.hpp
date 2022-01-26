#ifndef RDF4CPP_METALLIRIBACKEND_HPP
#define RDF4CPP_METALLIRIBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/handle/IRIBackendView.hpp>

#include <metall/container/string.hpp>
#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>

#include <compare>
#include <memory>
#include <string>
#include <string_view>

namespace Dice::node_store {
	class MetallIRIBackend {
		metall::container::string iri;

	public:
		using pointer_t = metall::manager::allocator_type<MetallIRIBackend>::pointer;
		explicit MetallIRIBackend(std::string_view iri, metall::manager::allocator_type<std::byte> const &allocator) noexcept;
		std::strong_ordering operator<=>(const MetallIRIBackend &other) const noexcept;
		std::strong_ordering operator<=>(pointer_t const &other) const noexcept;

		[[nodiscard]] std::string_view identifier() const noexcept;

		[[nodiscard]] std::string n_string() const noexcept;

		explicit operator rdf4cpp::rdf::storage::node::handle::IRIBackendView() const noexcept;
	};

	std::strong_ordering operator<=>(MetallIRIBackend::pointer_t const &self, MetallIRIBackend::pointer_t const &other) noexcept;
}// namespace Dice::node_store

namespace rdf4cpp::rdf::storage::node::handle {

	std::partial_ordering operator<=>(Dice::node_store::MetallIRIBackend::pointer_t const &self, IRIBackendView const &other) noexcept;
	bool operator==(Dice::node_store::MetallIRIBackend::pointer_t const &self, IRIBackendView const &other) noexcept;
}// namespace rdf4cpp::rdf::storage::node::handle

#endif//RDF4CPP_METALLIRIBACKEND_HPP
