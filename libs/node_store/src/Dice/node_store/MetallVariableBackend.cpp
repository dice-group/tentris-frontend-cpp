#include "MetallVariableBackend.hpp"

namespace Dice::node_store {

	MetallVariableBackend::MetallVariableBackend(std::string_view name, bool anonymous, metall::manager::allocator_type<std::byte> allocator) noexcept
		: name_(name, allocator), anonymous_(anonymous) {}
	std::strong_ordering MetallVariableBackend::operator<=>(MetallVariableBackend::pointer_t const &other) const noexcept {
		if (other != nullptr)
			return *this <=> *other;
		else
			return std::strong_ordering::greater;
	}
	std::string MetallVariableBackend::n_string() const noexcept {
		if (anonymous_)
			return "_:" + std::string{name_};
		else
			return "?" + std::string{name_};
	}
	bool MetallVariableBackend::is_anonymous() const noexcept {
		return anonymous_;
	}
	std::string_view MetallVariableBackend::name() const noexcept {
		return name_;
	}
	std::strong_ordering MetallVariableBackend::operator<=>(const MetallVariableBackend &other) const noexcept {
		return std::make_tuple(this->name(), this->is_anonymous()) <=> std::make_tuple(other.name(), other.is_anonymous());
	}
	MetallVariableBackend::operator rdf4cpp::rdf::storage::node::handle::VariableBackendView() const noexcept {
		return {.name = name(),
				.is_anonymous = is_anonymous()};
	}
	std::strong_ordering operator<=>(MetallVariableBackend::pointer_t const &self, MetallVariableBackend::pointer_t const &other) noexcept {
		return *self <=> *other;
	}
}// namespace Dice::node_store

namespace rdf4cpp::rdf::storage::node::handle {

	std::partial_ordering operator<=>(const Dice::node_store::MetallVariableBackend::pointer_t &self, const VariableBackendView &other) noexcept {
		return VariableBackendView{.name = self->name(), .is_anonymous = self->is_anonymous()} <=> other;
	}
	bool operator==(const Dice::node_store::MetallVariableBackend::pointer_t &self, const VariableBackendView &other) noexcept {
		return VariableBackendView{.name = self->name(), .is_anonymous = self->is_anonymous()} == other;
	}
}// namespace rdf4cpp::rdf::storage::node::handle