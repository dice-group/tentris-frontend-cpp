#ifndef RDF4CPP_METALLLITERALBACKEND_HPP
#define RDF4CPP_METALLLITERALBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/view/LiteralBackendView.hpp>
#include <rdf4cpp/rdf/storage/node/identifier/NodeID.hpp>

#include <Dice/node_store/metall_manager.hpp>

#include <compare>
#include <memory>
#include <string>
#include <string_view>

namespace Dice::node_store {

	class MetallLiteralBackend {
		rdf4cpp::rdf::storage::node::identifier::NodeID datatype_id_;
		metall_string lexical;
		metall_string lang_tag;

	public:
		using pointer_t = metall_manager::allocator_type<MetallLiteralBackend>::pointer;

		MetallLiteralBackend(std::string_view lexical, const rdf4cpp::rdf::storage::node::identifier::NodeID &dataType, std::string_view langTag, metall_manager::allocator_type<std::byte> const &allocator) noexcept;
		MetallLiteralBackend(rdf4cpp::rdf::storage::node::view::LiteralBackendView view, metall_manager::allocator_type<std::byte> const &allocator) noexcept;
		std::partial_ordering operator<=>(const MetallLiteralBackend &) const noexcept;
		std::partial_ordering operator<=>(MetallLiteralBackend::pointer_t const &other) const noexcept;

//		bool operator==(const MetallLiteralBackend &) const noexcept;
		bool operator==(const MetallLiteralBackend &other) const noexcept {
			return std::make_tuple(datatype_id(), lexical_form(), language_tag()) == std::make_tuple(other.datatype_id(), other.lexical_form(), other.language_tag());
		}

		[[nodiscard]] std::string_view lexical_form() const noexcept;

		[[nodiscard]] const rdf4cpp::rdf::storage::node::identifier::NodeID &datatype_id() const noexcept;

		[[nodiscard]] std::string_view language_tag() const noexcept;

		explicit operator rdf4cpp::rdf::storage::node::view::LiteralBackendView() const noexcept;
	};

	std::partial_ordering operator<=>(MetallLiteralBackend::pointer_t const &self, MetallLiteralBackend::pointer_t const &other) noexcept;


}// namespace Dice::node_store

namespace rdf4cpp::rdf::storage::node::view {
	std::partial_ordering operator<=>(Dice::node_store::MetallLiteralBackend::pointer_t const &self, LiteralBackendView const &other) noexcept;
	bool operator==(Dice::node_store::MetallLiteralBackend::pointer_t const &self, LiteralBackendView const &other) noexcept;
}// namespace rdf4cpp::rdf::storage::node::view

#endif//RDF4CPP_METALLLITERALBACKEND_HPP
