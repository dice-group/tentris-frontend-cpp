#ifndef TENTRIS_METALL_ALLOCATOR_HPP
#define TENTRIS_METALL_ALLOCATOR_HPP


#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif

#include <metall/basic_manager.hpp>

#include <metall/basic_manager.hpp>
#include <metall/container/string.hpp>


namespace Dice::node_store {

//	using metall_manager = metall::manager ;
	using metall_manager = metall::basic_manager<uint32_t, (1ULL << 28ULL)>;
	using metall_string = metall::container::basic_string<char, std::char_traits<char>, metall_manager::allocator_type<char>>;
}
#endif//TENTRIS_METALL_ALLOCATOR_HPP
