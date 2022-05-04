#include "MetallBNodeBackend.hpp"

namespace Dice::node_store {

	MetallBNodeBackend::MetallBNodeBackend(std::string_view identifier, metall_manager::allocator_type<std::byte> const &allocator) noexcept
		: identifier_(identifier, allocator) {}
	MetallBNodeBackend::MetallBNodeBackend(rdf4cpp::rdf::storage::node::view::BNodeBackendView view, metall_manager::allocator_type<std::byte> const &allocator) noexcept :
			identifier_(view.identifier, allocator){	}
	std::partial_ordering MetallBNodeBackend::operator<=>(MetallBNodeBackend::pointer_t const &other) const noexcept {
		if (other != nullptr)
			return *this <=> *other;
		else
			return std::partial_ordering::greater;
	}
	std::string MetallBNodeBackend::n_string() const noexcept {
		return "_:" + std::string{identifier_};
	}
	std::string_view MetallBNodeBackend::identifier() const noexcept {
		return identifier_;
	}
	std::weak_ordering MetallBNodeBackend::operator<=>(const MetallBNodeBackend &other) const noexcept {
		return this->identifier() <=> other.identifier();
	}
	MetallBNodeBackend::operator rdf4cpp::rdf::storage::node::view::BNodeBackendView() const noexcept {
		return {.identifier = identifier()};
	}
	std::partial_ordering operator<=>(const MetallBNodeBackend::pointer_t &self, const MetallBNodeBackend::pointer_t &other) noexcept {
		return *self <=> *other;
	}
}// namespace Dice::node_store

namespace rdf4cpp::rdf::storage::node::view {

	std::partial_ordering operator<=>(const Dice::node_store::MetallBNodeBackend::pointer_t &self, const BNodeBackendView &other) noexcept {
		return BNodeBackendView{.identifier = self->identifier()} <=> other;
	}
	bool operator==(const Dice::node_store::MetallBNodeBackend::pointer_t &self, const BNodeBackendView &other) noexcept {
		return BNodeBackendView{.identifier = self->identifier()} == other;
	}
}// namespace rdf4cpp::rdf::storage::node::view
