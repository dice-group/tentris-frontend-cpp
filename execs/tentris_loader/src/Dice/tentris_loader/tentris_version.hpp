#ifndef TENTRIS_VERSION_HPP
#define TENTRIS_VERSION_HPP

#include <array>

namespace Dice::tentris {
	inline constexpr const char name[] = "tentris";
	inline constexpr const char version[] = "1.2.0";
	inline constexpr std::array<int, 3> version_tuple = {1, 2, 0};
}// namespace Dice::tentris

#endif//TENTRIS_VERSION_HPP
