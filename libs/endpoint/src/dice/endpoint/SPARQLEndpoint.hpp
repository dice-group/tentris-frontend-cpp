#ifndef TENTRIS_SPARQLENDPOINT_HPP
#define TENTRIS_SPARQLENDPOINT_HPP

#include <restinio/all.hpp>
#include <taskflow/taskflow.hpp>

#include <dice/tentris/param_allocator.hpp>
#include <dice/triplestore/TripleStore.hpp>

namespace dice::endpoint {

	class SPARQLEndpoint {

		tf::Executor &executor_;

		triplestore::TripleStore &triplestore_;

		std::chrono::seconds timeout_duration_;

	public:
		SPARQLEndpoint(tf::Executor &executor, triplestore::TripleStore &triplestore, std::chrono::seconds timeoutDuration);

		restinio::request_handling_status_t operator()(
				restinio::request_handle_t const &req,
				restinio::router::route_params_t params);
	};

}// namespace dice::endpoint
#endif//TENTRIS_SPARQLENDPOINT_HPP
