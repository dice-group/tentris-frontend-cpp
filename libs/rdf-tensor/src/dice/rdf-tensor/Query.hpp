#ifndef TENTRIS_QUERY_HPP
#define TENTRIS_QUERY_HPP

#include <dice/query.hpp>
#include <dice/sparql.hpp>

namespace dice::rdf_tensor {
	using key_part_type = dice::sparql::detail::key_part_type;
	using htt_t = dice::sparql::detail::htt_t;
	using allocator_type = dice::sparql::detail::allocator_type;
	using metall_manager = dice::sparql::detail::metall_manager;
	using SolutionMapping = dice::query::CountedEntry<htt_t>;
	using QueryEvalaution = dice::query::Evaluation;
	using Query = dice::query::Query<htt_t, allocator_type>;
	using SPARQLQuery = dice::sparql::SPARQLQuery;
	using HypertrieContext = dice::hypertrie::HypertrieContext<htt_t, allocator_type>;
	using BoolHypertrie = dice::hypertrie::Hypertrie<htt_t, allocator_type>;
	using const_BoolHypertrie = dice::hypertrie::const_Hypertrie<htt_t, allocator_type>;
	using HypertrieBulkInserter = dice::hypertrie::BulkInserter<htt_t, allocator_type, hypertrie::BulkUpdaterSyncness::Async>;
	using HypertrieSyncBulkInserter = dice::hypertrie::BulkInserter<htt_t, allocator_type, hypertrie::BulkUpdaterSyncness::Sync>;
	using HypertrieContext_ptr = dice::hypertrie::HypertrieContext_ptr<htt_t, allocator_type>;
	using SliceKey = dice::hypertrie::SliceKey<htt_t>;
	using Key = dice::hypertrie::Key<htt_t>;
}// namespace dice::rdf-tensor

#endif//TENTRIS_QUERY_HPP
