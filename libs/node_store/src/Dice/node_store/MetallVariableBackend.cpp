#include "MetallVariableBackend.hpp"

namespace Dice::node_store {

	MetallVariableBackend::MetallVariableBackend(std::string_view name, bool anonymous, metall_manager::allocator_type<std::byte> const &allocator) noexcept
		: name_(name, allocator), anonymous_(anonymous) {}
	MetallVariableBackend::MetallVariableBackend(rdf4cpp::rdf::storage::node::view::VariableBackendView view, metall_manager::allocator_type<std::byte> const &allocator) noexcept
		: name_(view.name, allocator), anonymous_(view.is_anonymous) {}
	std::partial_ordering MetallVariableBackend::operator<=>(MetallVariableBackend::pointer_t const &other) const noexcept {
		if (other != nullptr)
			return *this <=> *other;
		else
			return std::partial_ordering::greater;
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
	std::partial_ordering MetallVariableBackend::operator<=>(const MetallVariableBackend &other) const noexcept {
		return std::make_tuple(this->name(), this->is_anonymous()) <=> std::make_tuple(other.name(), other.is_anonymous());
	}
	MetallVariableBackend::operator rdf4cpp::rdf::storage::node::view::VariableBackendView() const noexcept {
		return {.name = name(),
				.is_anonymous = is_anonymous()};
	}
	std::partial_ordering operator<=>(MetallVariableBackend::pointer_t const &self, MetallVariableBackend::pointer_t const &other) noexcept {
		return *self <=> *other;
	}
}// namespace Dice::node_store

namespace rdf4cpp::rdf::storage::node::view {

	std::partial_ordering operator<=>(const Dice::node_store::MetallVariableBackend::pointer_t &self, const VariableBackendView &other) noexcept {
		return VariableBackendView{.name = self->name(), .is_anonymous = self->is_anonymous()} <=> other;
	}
	bool operator==(const Dice::node_store::MetallVariableBackend::pointer_t &self, const VariableBackendView &other) noexcept {
		return VariableBackendView{.name = self->name(), .is_anonymous = self->is_anonymous()} == other;
	}
}// namespace rdf4cpp::rdf::storage::node::view