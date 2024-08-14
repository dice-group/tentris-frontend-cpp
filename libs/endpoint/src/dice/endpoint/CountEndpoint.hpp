#ifndef TENTRIS_COUNTENDPOINT_HPP
#define TENTRIS_COUNTENDPOINT_HPP

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#include <restinio/all.hpp>
#include <taskflow/taskflow.hpp>

#include <dice/triple-store/TripleStore.hpp>

#include <dice/endpoint/EndointCfg.hpp>
#include <dice/endpoint/SparqlQueryCache.hpp>

namespace dice::endpoint {

	class CountEndpoint {

		tf::Executor &executor_;

		triple_store::TripleStore &triplestore_;

		SparqlQueryCache &sparql_query_cache_;

		EndpointCfg cfg_;

	public:
		CountEndpoint(tf::Executor &executor, triple_store::TripleStore &triplestore, SparqlQueryCache &sparql_query_cache, EndpointCfg const &endpoint_cfg);

		restinio::request_handling_status_t operator()(
				restinio::request_handle_t req,
				restinio::router::route_params_t params);
	};

}// namespace dice::endpoint
#endif//TENTRIS_COUNTENDPOINT_HPP
