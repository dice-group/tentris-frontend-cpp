#ifndef RDF4CPP_METALLBNODEBACKEND_HPP
#define RDF4CPP_METALLBNODEBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/view/BNodeBackendView.hpp>

#include <Dice/node_store/metall_manager.hpp>

#include <compare>
#include <memory>
#include <string>
#include <string_view>

namespace Dice::node_store {

	class MetallBNodeBackend {
		metall_string identifier_;

	public:
		using pointer_t = metall_manager::allocator_type<MetallBNodeBackend>::pointer;
		explicit MetallBNodeBackend(std::string_view identifier, metall_manager::allocator_type<std::byte> const &allocator) noexcept;
		MetallBNodeBackend(rdf4cpp::rdf::storage::node::view::BNodeBackendView view, metall_manager::allocator_type<std::byte> const &allocator) noexcept;
		std::weak_ordering operator<=>(const MetallBNodeBackend &other) const noexcept;
		bool operator==(const MetallBNodeBackend &other) const noexcept = default;
		std::partial_ordering operator<=>(MetallBNodeBackend::pointer_t const &other) const noexcept;
		[[nodiscard]] std::string n_string() const noexcept;
		[[nodiscard]] std::string_view identifier() const noexcept;

		explicit operator rdf4cpp::rdf::storage::node::view::BNodeBackendView() const noexcept;
	};

	std::partial_ordering operator<=>(MetallBNodeBackend::pointer_t const &self, MetallBNodeBackend::pointer_t const &other) noexcept;
}// namespace Dice::node_store

namespace rdf4cpp::rdf::storage::node::view {

	std::partial_ordering operator<=>(Dice::node_store::MetallBNodeBackend::pointer_t const &self, BNodeBackendView const &other) noexcept;
	bool operator==(Dice::node_store::MetallBNodeBackend::pointer_t const &self, BNodeBackendView const &other) noexcept;
}// namespace rdf4cpp::rdf::storage::node::view

#endif//RDF4CPP_METALLBNODEBACKEND_HPP
