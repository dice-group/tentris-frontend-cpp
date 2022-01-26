#ifndef RDF4CPP_METALLBNODEBACKEND_HPP
#define RDF4CPP_METALLBNODEBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/handle/BNodeBackendView.hpp>

#include <metall/container/string.hpp>
#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>

#include <compare>
#include <memory>
#include <string>
#include <string_view>

namespace Dice::node_storage {

	class MetallBNodeBackend {
		metall::container::string identifier_;

	public:
		using pointer_t = metall::manager::allocator_type<MetallBNodeBackend>::pointer;
		explicit MetallBNodeBackend(std::string_view identifier, metall::manager::allocator_type<std::byte> const &allocator) noexcept;
		auto operator<=>(const MetallBNodeBackend &other) const noexcept {
			return this->identifier() <=> other.identifier();
		};
		std::strong_ordering operator<=>(MetallBNodeBackend::pointer_t const &other) const noexcept;
		[[nodiscard]] std::string n_string() const noexcept;
		[[nodiscard]] std::string_view identifier() const noexcept;

		explicit operator rdf4cpp::rdf::storage::node::handle::BNodeBackendView() const noexcept {
			return {.identifier = identifier()};
		}
	};

//	inline auto operator==(MetallBNodeBackend::pointer_t const &self, rdf4cpp::rdf::storage::node::BNodeBackendHandle const &other) noexcept {
//		return rdf4cpp::rdf::storage::node::BNodeBackendHandle(*self) == other;
//	}

	std::strong_ordering operator<=>(MetallBNodeBackend::pointer_t const &self, MetallBNodeBackend::pointer_t const &other) noexcept;
}// namespace Dice::node_storage


#endif//RDF4CPP_METALLBNODEBACKEND_HPP
