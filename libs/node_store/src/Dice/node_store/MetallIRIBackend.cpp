#include "MetallIRIBackend.hpp"

namespace Dice::node_storage {

MetallIRIBackend::MetallIRIBackend(std::string_view iri,metall::manager::allocator_type<std::byte> const &allocator) noexcept : iri(iri, allocator) {}
std::strong_ordering MetallIRIBackend::operator<=>(MetallIRIBackend::pointer_t const &other) const noexcept {
    if (other)
        return *this <=> *other;
    else
        return std::strong_ordering::greater;
}
std::string MetallIRIBackend::n_string() const noexcept {
    return "<" + iri + ">";
}
std::string_view MetallIRIBackend::identifier() const noexcept {
    return iri;
}
std::strong_ordering operator<=>(const MetallIRIBackend::pointer_t &self, const MetallIRIBackend::pointer_t &other) noexcept {
    return *self <=> *other;
}
}  // namespace rdf4cpp::rdf::storage::node