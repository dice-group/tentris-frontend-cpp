#ifndef TENTRIS_QUERY_HPP
#define TENTRIS_QUERY_HPP

#include "Dice/rdf_tensor/HypertrieTrait.hpp"
#include "Dice/rdf_tensor/metall_manager.hpp"
#include <Dice/query.hpp>

namespace Dice::rdf_tensor {
	using COUNTED_t = std::size_t;
	using Entry = Dice::query::Entry<COUNTED_t, htt_t>;
	using DISTINCT_t = bool;
	using UncountedEntry = Dice::query::Entry<DISTINCT_t, htt_t>;
	using Query = Dice::query::Query<htt_t, allocator_type>;
}// namespace Dice::rdf_tensor

#endif//TENTRIS_QUERY_HPP
