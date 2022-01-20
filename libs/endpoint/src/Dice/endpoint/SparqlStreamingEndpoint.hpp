#ifndef TENTRIS_SPARQLSTREAMINGENDPOINT_HPP
#define TENTRIS_SPARQLSTREAMINGENDPOINT_HPP

#include <restinio/all.hpp>
#include <taskflow/taskflow.hpp>

#include <Dice/triple_store/TripleStore.hpp>


namespace Dice::endpoint {

	class SPARQLStreamingEndpoint {
		tf::Executor &executor_;

		triple_store::TripleStore &triplestore_;

		std::chrono::seconds timeout_duration_;

	public:
		SPARQLStreamingEndpoint(tf::Executor &executor, triple_store::TripleStore &triplestore, std::chrono::seconds timeoutDuration);

		restinio::request_handling_status_t operator()(
				restinio::request_handle_t req,
				restinio::router::route_params_t params);
	};
}// namespace Dice::endpoint

#endif//TENTRIS_SPARQLSTREAMINGENDPOINT_HPP
