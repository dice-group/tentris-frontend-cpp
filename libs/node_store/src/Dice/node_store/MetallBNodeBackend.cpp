#include "MetallBNodeBackend.hpp"

namespace Dice::node_storage {

MetallBNodeBackend::MetallBNodeBackend(std::string_view identifier, metall::manager::allocator_type<std::byte> const &allocator) noexcept
    : identifier_(identifier, allocator) {}
std::strong_ordering MetallBNodeBackend::operator<=>(MetallBNodeBackend::pointer_t const &other) const noexcept {
    if (other != nullptr)
        return *this <=> *other;
    else
        return std::strong_ordering::greater;
}
std::string MetallBNodeBackend::n_string() const noexcept {
    return "_:" + std::string{identifier_};
}
std::string_view MetallBNodeBackend::identifier() const noexcept {
    return identifier_;
}
std::strong_ordering operator<=>(const MetallBNodeBackend::pointer_t &self, const MetallBNodeBackend::pointer_t &other) noexcept {
    return *self <=> *other;
}
}  // namespace rdf4cpp::rdf::storage::node