#ifndef TENTRIS_METALL_MANAGER_HPP
#define TENTRIS_METALL_MANAGER_HPP

#include <metall/basic_manager.hpp>

namespace Dice::rdf_tensor {
	using metall_manager = metall::basic_manager<uint32_t, (1ULL << 28ULL)>;
	using allocator_type = metall_manager::allocator_type<std::byte>;
}// namespace Dice::rdf_tensor

#endif//TENTRIS_METALL_MANAGER_HPP
