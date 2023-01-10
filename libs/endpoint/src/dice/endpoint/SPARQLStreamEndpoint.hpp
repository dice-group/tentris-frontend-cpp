#ifndef TENTRIS_SPARQLSTREAMENDPOINT_HPP
#define TENTRIS_SPARQLSTREAMENDPOINT_HPP

#include <restinio/all.hpp>
#include <taskflow/taskflow.hpp>

#include <dice/node-store/metall_manager.hpp>
#include <dice/triple-store/TripleStore.hpp>

namespace dice::endpoint {

	class SPARQLStreamEndpoint {

		tf::Executor &executor_;

		triple_store::TripleStore &triplestore_;

		std::chrono::seconds timeout_duration_;

	public:
		SPARQLStreamEndpoint(tf::Executor &executor, triple_store::TripleStore &triplestore, std::chrono::seconds timeoutDuration);

		restinio::request_handling_status_t operator()(
				restinio::request_handle_t const &req,
				restinio::router::route_params_t params);
	};

}// namespace dice::endpoint

#endif//TENTRIS_SPARQLSTREAMENDPOINT_HPP
