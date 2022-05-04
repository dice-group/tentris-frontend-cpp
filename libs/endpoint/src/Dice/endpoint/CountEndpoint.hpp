#ifndef TENTRIS_COUNTENDPOINT_HPP
#define TENTRIS_COUNTENDPOINT_HPP

#include <restinio/all.hpp>
#include <taskflow/taskflow.hpp>

#include <Dice/triple_store/TripleStore.hpp>
#include <Dice/node_store/metall_manager.hpp>

#include <Dice/endpoint/SparqlQueryCache.hpp>

namespace Dice::endpoint {

	class CountEndpoint {

		tf::Executor &executor_;

		triple_store::TripleStore<typename Dice::node_store::metall_manager::allocator_type<std::byte>> &triplestore_;

		SparqlQueryCache &sparql_query_cache_;

		std::chrono::seconds timeout_duration_;

	public:
		CountEndpoint(tf::Executor &executor, triple_store::TripleStore<typename Dice::node_store::metall_manager::allocator_type<std::byte>> &triplestore, SparqlQueryCache &sparql_query_cache, std::chrono::seconds timeoutDuration);

		restinio::request_handling_status_t operator()(
				restinio::request_handle_t req,
				restinio::router::route_params_t params);
	};

}// namespace Dice::endpoint
#endif//TENTRIS_COUNTENDPOINT_HPP
