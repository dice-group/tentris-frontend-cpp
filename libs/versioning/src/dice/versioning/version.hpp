#ifndef TENTRIS_VERSION_HPP
#define TENTRIS_VERSION_HPP

#include <array>

namespace dice::tentris {
	inline constexpr const char name[] = "tentris";
	inline constexpr const char version[] = "1.3.0";
	inline constexpr std::array<int, 3> version_tuple = {1, 3, 0};
	inline constexpr const char rdf4cpp_version[] = ""; // todo: remove once rdf4cpp has its own version header
}// namespace dice::tentris

#endif//TENTRIS_VERSION_HPP
