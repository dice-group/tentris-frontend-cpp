#ifndef RDF4CPP_METALLLITERALBACKEND_HPP
#define RDF4CPP_METALLLITERALBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/handle/LiteralBackendView.hpp>
#include <rdf4cpp/rdf/storage/node/identifier/NodeID.hpp>

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

	class MetallLiteralBackend {
		rdf4cpp::rdf::storage::node::identifier::NodeID datatype_id_;
		metall::container::string lexical;
		metall::container::string lang_tag;

	public:
		using pointer_t = metall::manager::allocator_type<MetallLiteralBackend>::pointer;

		MetallLiteralBackend(std::string_view lexical, const rdf4cpp::rdf::storage::node::identifier::NodeID &dataType, std::string_view langTag, metall::manager::allocator_type<std::byte> const &allocator) noexcept;
		std::strong_ordering operator<=>(const MetallLiteralBackend &) const noexcept;
		std::strong_ordering operator<=>(MetallLiteralBackend::pointer_t const &other) const noexcept;

		bool operator==(const MetallLiteralBackend &) const noexcept;

		[[nodiscard]] std::string_view lexical_form() const noexcept;

		[[nodiscard]] const rdf4cpp::rdf::storage::node::identifier::NodeID &datatype_id() const noexcept;

		[[nodiscard]] std::string_view language_tag() const noexcept;

		explicit operator rdf4cpp::rdf::storage::node::handle::LiteralBackendView() const noexcept;
	};

	std::strong_ordering operator<=>(MetallLiteralBackend::pointer_t const &self, MetallLiteralBackend::pointer_t const &other) noexcept;


}// namespace Dice::node_store

namespace rdf4cpp::rdf::storage::node::handle {
	std::partial_ordering operator<=>(Dice::node_store::MetallLiteralBackend::pointer_t const &self, LiteralBackendView const &other) noexcept;
	bool operator==(Dice::node_store::MetallLiteralBackend::pointer_t const &self, LiteralBackendView const &other) noexcept;
}// namespace rdf4cpp::rdf::storage::node::handle

#endif//RDF4CPP_METALLLITERALBACKEND_HPP
