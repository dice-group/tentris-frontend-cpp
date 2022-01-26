#include "MetallLiteralBackend.hpp"
#include <tuple>
namespace Dice::node_store {

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
	}
	MetallLiteralBackend::operator rdf4cpp::rdf::storage::node::handle::LiteralBackendView() const noexcept {
		return {.datatype_id = datatype_id(),
				.lexical_form = lexical_form(),
				.language_tag = language_tag()};
	};
	std::strong_ordering operator<=>(const MetallLiteralBackend::pointer_t &self, const MetallLiteralBackend::pointer_t &other) noexcept {
		return *self <=> *other;
	}
}// namespace Dice::node_store

namespace rdf4cpp::rdf::storage::node::handle {
	std::partial_ordering operator<=>(const Dice::node_store::MetallLiteralBackend::pointer_t &self, const LiteralBackendView &other) noexcept {
		return LiteralBackendView{.datatype_id = self->datatype_id(), .lexical_form = self->lexical_form(), .language_tag = self->language_tag()} <=> other;
	}
	bool operator==(const Dice::node_store::MetallLiteralBackend::pointer_t &self, const LiteralBackendView &other) noexcept {
		return LiteralBackendView{.datatype_id = self->datatype_id(), .lexical_form = self->lexical_form(), .language_tag = self->language_tag()} == other;
	}
}// namespace rdf4cpp::rdf::storage::node::handle