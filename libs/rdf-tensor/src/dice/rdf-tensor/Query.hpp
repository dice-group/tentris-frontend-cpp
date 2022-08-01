#ifndef TENTRIS_QUERY_HPP
#define TENTRIS_QUERY_HPP

#include "dice/rdf-tensor/HypertrieTrait.hpp"
#include "dice/rdf-tensor/metall_manager.hpp"
#include <dice/query.hpp>

namespace dice::rdf_tensor {
	using COUNTED_t = std::size_t;
	using Entry = dice::query::Entry<COUNTED_t, htt_t>;
	using Query = dice::query::Query<htt_t, allocator_type>;
	using QueryEvaluation = dice::query::Evaluation;
	using operand_desc = dice::query::operand_desc;
	using Expression = dice::query::Expression<htt_t>;
	using FilterExpression = dice::query::FilterExpression<htt_t>;
}// namespace dice::rdf-tensor

#endif//TENTRIS_QUERY_HPP
