#include "MetallIRIBackend.hpp"

namespace Dice::node_store {

	MetallIRIBackend::MetallIRIBackend(std::string_view iri, metall_manager::allocator_type<std::byte> const &allocator) noexcept : iri(iri, allocator) {}
	MetallIRIBackend::MetallIRIBackend(rdf4cpp::rdf::storage::node::view::IRIBackendView iri, metall_manager::allocator_type<std::byte> const &allocator) noexcept : iri(iri.identifier, allocator) {}
	std::partial_ordering MetallIRIBackend::operator<=>(MetallIRIBackend::pointer_t const &other) const noexcept {
		if (other)
			return *this <=> *other;
		else
			return std::partial_ordering::greater;
	}
	std::string MetallIRIBackend::n_string() const noexcept {
		return "<" + std::string{iri} + ">";
	}
	std::string_view MetallIRIBackend::identifier() const noexcept {
		return iri;
	}
	std::partial_ordering MetallIRIBackend::operator<=>(const MetallIRIBackend &other) const noexcept {
		return this->identifier() <=> other.identifier();
	}
	MetallIRIBackend::operator rdf4cpp::rdf::storage::node::view::IRIBackendView() const noexcept {
		return {.identifier = identifier()};
	}
	std::partial_ordering operator<=>(const MetallIRIBackend::pointer_t &self, const MetallIRIBackend::pointer_t &other) noexcept {
		return *self <=> *other;
	}
}// namespace Dice::node_store

namespace rdf4cpp::rdf::storage::node::view {

	std::partial_ordering operator<=>(const Dice::node_store::MetallIRIBackend::pointer_t &self, const IRIBackendView &other) noexcept {
		return IRIBackendView{.identifier = self->identifier()} <=> other;
	}
	bool operator==(const Dice::node_store::MetallIRIBackend::pointer_t &self, const IRIBackendView &other) noexcept {
		return IRIBackendView{.identifier = self->identifier()} == other;
	}
}// namespace rdf4cpp::rdf::storage::node::view