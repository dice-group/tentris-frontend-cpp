#ifndef TENTRIS_RDFEINSUM_HPP
#define TENTRIS_RDFEINSUM_HPP

#include "Dice/rdf_tensor/HypertrieTrait.hpp"
#include <Dice/einsum.hpp>

namespace Dice::rdf_tensor {
	using COUNTED_t = std::size_t;
	using EinsumEntry = Dice::einsum::Entry<COUNTED_t, htt_t>;
	using DISTINCT_t = bool;
	using UncountedEinsumEntry = Dice::einsum::Entry<DISTINCT_t, htt_t>;
}// namespace Dice::rdf_tensor

#endif//TENTRIS_RDFEINSUM_HPP
