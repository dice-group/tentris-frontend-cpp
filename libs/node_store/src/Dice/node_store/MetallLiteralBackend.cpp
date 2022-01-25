#include "MetallLiteralBackend.hpp"
#include <tuple>
namespace Dice::node_storage {

	MetallLiteralBackend::MetallLiteralBackend(std::string_view lexical, const rdf4cpp::rdf::storage::node::identifier::NodeID &dataType, std::string_view langTag, metall::manager::allocator_type<std::byte> const &allocator) noexcept
		: datatype_id_(dataType),
		  lexical(lexical, allocator),
		  lang_tag(langTag, allocator) {}
	std::strong_ordering MetallLiteralBackend::operator<=>(const MetallLiteralBackend &other) const noexcept {
		return std::make_tuple(this->datatype_id_.node_id(), this->lexical_form(), this->language_tag()) <=> std::make_tuple(other.datatype_id_.node_id(), other.lexical_form(), other.language_tag());
	}

	bool MetallLiteralBackend::operator==(const MetallLiteralBackend &other) const noexcept {
		return std::make_tuple(this->datatype_id_.node_id(), this->lexical_form(), this->language_tag()) == std::make_tuple(other.datatype_id_.node_id(), other.lexical_form(), other.language_tag());
	}
	std::strong_ordering MetallLiteralBackend::operator<=>(const MetallLiteralBackend::pointer_t &other) const noexcept {
		if (other)
			return *this <=> *other;
		else
			return std::strong_ordering::greater;
	}
	std::string_view MetallLiteralBackend::language_tag() const noexcept {
		return lang_tag;
	}
	const rdf4cpp::rdf::storage::node::identifier::NodeID &MetallLiteralBackend::datatype_id() const noexcept {
		return datatype_id_;
	}
	std::string_view MetallLiteralBackend::lexical_form() const noexcept {
		return lexical;
	};
	std::strong_ordering operator<=>(const MetallLiteralBackend::pointer_t &self, const MetallLiteralBackend::pointer_t &other) noexcept {
		return *self <=> *other;
	}
}// namespace rdf4cpp::rdf::storage::node