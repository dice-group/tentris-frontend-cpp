#ifndef RDF4CPP_METALLIRIBACKEND_HPP
#define RDF4CPP_METALLIRIBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/handle/IRIBackendView.hpp>

#include <metall/container/string.hpp>
#include <metall/metall.hpp>

#include <compare>
#include <memory>
#include <string>
#include <string_view>

namespace Dice::node_storage {
	class MetallIRIBackend {
		metall::container::string iri;

	public:
		using pointer_t = metall::manager::allocator_type<MetallIRIBackend>::pointer;
		explicit MetallIRIBackend(std::string_view iri, metall::manager::allocator_type<std::byte> const &allocator) noexcept;
		auto operator<=>(const MetallIRIBackend &other) const noexcept {
			return this->identifier() <=> other.identifier();
		}
		std::strong_ordering operator<=>(pointer_t const &other) const noexcept;

		[[nodiscard]] std::string_view identifier() const noexcept;

		[[nodiscard]] std::string n_string() const noexcept;

		explicit operator rdf4cpp::rdf::storage::node::handle::IRIBackendView() const noexcept {
			return {.identifier = identifier()};
		}
	};

//	inline auto operator==(MetallIRIBackend::pointer_t const &self, rdf4cpp::rdf::storage::node::IRIBackendHandle const &other) noexcept {
//		return rdf4cpp::rdf::storage::node::IRIBackendHandle(*self) == other;
//	}
	std::strong_ordering operator<=>(MetallIRIBackend::pointer_t const &self, MetallIRIBackend::pointer_t const &other) noexcept;
}// namespace Dice::node_storage

#endif//RDF4CPP_METALLIRIBACKEND_HPP
