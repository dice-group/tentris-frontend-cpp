#ifndef TENTRIS_RDFTENSOR_HPP
#define TENTRIS_RDFTENSOR_HPP

#include "Dice/rdf_tensor/HypertrieTrait.hpp"
#include "Dice/rdf_tensor/metall_manager.hpp"

namespace Dice::rdf_tensor {
	using HypertrieContext = Dice::hypertrie::HypertrieContext<htt_t, allocator_type>;
	using BoolHypertrie = Dice::hypertrie::Hypertrie<htt_t, allocator_type>;
	using const_BoolHypertrie = Dice::hypertrie::const_Hypertrie<htt_t, allocator_type>;
	using HypertrieBulkInserter = Dice::hypertrie::BulkInserter<htt_t, allocator_type>;
	using HypertrieContext_ptr = Dice::hypertrie::HypertrieContext_ptr<htt_t, allocator_type>;
}// namespace Dice::rdf_tensor
#endif//TENTRIS_RDFTENSOR_HPP
