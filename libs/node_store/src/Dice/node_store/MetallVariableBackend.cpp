#include "MetallVariableBackend.hpp"

namespace Dice::node_storage {

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
        return "_:" + name_;
    else
        return "?" + name_;
}
bool MetallVariableBackend::is_anonymous() const noexcept {
    return anonymous_;
}
std::string_view MetallVariableBackend::name() const noexcept {
    return name_;
}
std::strong_ordering operator<=>(MetallVariableBackend::pointer_t const &self, MetallVariableBackend::pointer_t const &other) noexcept {
    return *self <=> *other;
}
}  // namespace rdf4cpp::rdf::storage::node