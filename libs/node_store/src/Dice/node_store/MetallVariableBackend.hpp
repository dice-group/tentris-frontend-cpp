#ifndef RDF4CPP_METALLVARIABLEBACKEND_HPP
#define RDF4CPP_METALLVARIABLEBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/handle/VariableBackendView.hpp>

#include <metall/container/string.hpp>
#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>

#include <compare>
#include <memory>
#include <string_view>

namespace Dice::node_store {

	class MetallVariableBackend {
		metall::container::string name_;
		bool anonymous_;

	public:
		using pointer_t = metall::manager::allocator_type<MetallVariableBackend>::pointer;
		explicit MetallVariableBackend(std::string_view name, bool anonymous, metall::manager::allocator_type<std::byte> allocator) noexcept;
		std::strong_ordering operator<=>(MetallVariableBackend const &other) const noexcept;
		std::strong_ordering operator<=>(pointer_t const &other) const noexcept;

		[[nodiscard]] std::string n_string() const noexcept;

		[[nodiscard]] bool is_anonymous() const noexcept;

		[[nodiscard]] std::string_view name() const noexcept;

		explicit operator rdf4cpp::rdf::storage::node::handle::VariableBackendView() const noexcept;
	};

	std::strong_ordering operator<=>(MetallVariableBackend::pointer_t const &self, MetallVariableBackend::pointer_t const &other) noexcept;
}// namespace Dice::node_store


namespace rdf4cpp::rdf::storage::node::handle {

	std::partial_ordering operator<=>(Dice::node_store::MetallVariableBackend::pointer_t const &self, VariableBackendView const &other) noexcept;
	bool operator==(Dice::node_store::MetallVariableBackend::pointer_t const &self, VariableBackendView const &other) noexcept;
}// namespace rdf4cpp::rdf::storage::node::handle

#endif//RDF4CPP_METALLVARIABLEBACKEND_HPP
