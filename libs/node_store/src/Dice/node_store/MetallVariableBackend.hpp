#ifndef RDF4CPP_METALLVARIABLEBACKEND_HPP
#define RDF4CPP_METALLVARIABLEBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/view/VariableBackendView.hpp>

#include <Dice/node_store/metall_manager.hpp>

#include <compare>
#include <memory>
#include <string_view>

namespace Dice::node_store {

	class MetallVariableBackend {
		metall_string name_;
		bool anonymous_;

	public:
		using pointer_t = metall_manager::allocator_type<MetallVariableBackend>::pointer;
		explicit MetallVariableBackend(std::string_view name, bool anonymous, metall_manager::allocator_type<std::byte> const &allocator) noexcept;
		explicit MetallVariableBackend(rdf4cpp::rdf::storage::node::view::VariableBackendView, metall_manager::allocator_type<std::byte> const &allocator) noexcept;
		std::partial_ordering operator<=>(MetallVariableBackend const &other) const noexcept;
		std::partial_ordering operator<=>(pointer_t const &other) const noexcept;
		bool operator==(const MetallVariableBackend &other) const noexcept = default;

		[[nodiscard]] std::string n_string() const noexcept;

		[[nodiscard]] bool is_anonymous() const noexcept;

		[[nodiscard]] std::string_view name() const noexcept;

		explicit operator rdf4cpp::rdf::storage::node::view::VariableBackendView() const noexcept;
	};

	std::partial_ordering operator<=>(MetallVariableBackend::pointer_t const &self, MetallVariableBackend::pointer_t const &other) noexcept;
}// namespace Dice::node_store


namespace rdf4cpp::rdf::storage::node::view {

	std::partial_ordering operator<=>(Dice::node_store::MetallVariableBackend::pointer_t const &self, VariableBackendView const &other) noexcept;
	bool operator==(Dice::node_store::MetallVariableBackend::pointer_t const &self, VariableBackendView const &other) noexcept;
}// namespace rdf4cpp::rdf::storage::node::view

#endif//RDF4CPP_METALLVARIABLEBACKEND_HPP
