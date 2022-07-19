#ifndef TENTRIS_QUERY_HPP
#define TENTRIS_QUERY_HPP

#include "Dice/rdf_tensor/HypertrieTrait.hpp"
#include "Dice/rdf_tensor/metall_manager.hpp"
#include <Dice/query.hpp>

namespace Dice::rdf_tensor {
	using COUNTED_t = std::size_t;
	using Entry = Dice::query::Entry<COUNTED_t, htt_t>;
	using Query = Dice::query::Query<htt_t, allocator_type>;
	using QueryEvaluation = Dice::query::Evaluation;
	using operand_desc = Dice::query::operand_desc;
	using Expression = Dice::query::Expression<htt_t>;
	using FilterExpression = Dice::query::FilterExpression<htt_t>;
}// namespace Dice::rdf_tensor

#endif//TENTRIS_QUERY_HPP
