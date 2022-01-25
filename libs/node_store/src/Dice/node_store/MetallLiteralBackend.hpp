#ifndef RDF4CPP_METALLLITERALBACKEND_HPP
#define RDF4CPP_METALLLITERALBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/identifier/NodeID.hpp>
#include <rdf4cpp/rdf/storage/node/handle/LiteralBackendView.hpp>

#include <metall/container/string.hpp>
#include <metall/metall.hpp>

#include <compare>
#include <memory>
#include <string>
#include <string_view>

namespace Dice::node_storage {

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

		explicit operator rdf4cpp::rdf::storage::node::handle::LiteralBackendView() const noexcept {
			return {.datatype_id = datatype_id(),
					.lexical_form = lexical_form(),
					.language_tag = language_tag()};
		}
	};

//	inline auto operator==(MetallLiteralBackend::pointer_t  const &self, rdf4cpp::rdf::storage::node::LiteralBackendHandle const &other) noexcept{
//		return rdf4cpp::rdf::storage::node::LiteralBackendHandle(*self) == other;
//	}

	std::strong_ordering operator<=>(MetallLiteralBackend::pointer_t const &self, MetallLiteralBackend::pointer_t const &other) noexcept;
}// namespace rdf4cpp::rdf::storage::node

#endif//RDF4CPP_METALLLITERALBACKEND_HPP
