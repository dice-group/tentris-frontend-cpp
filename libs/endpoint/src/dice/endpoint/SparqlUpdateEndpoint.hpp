#ifndef TENTRIS_SPARQLUPDATEENDPOINT_HPP
#define TENTRIS_SPARQLUPDATEENDPOINT_HPP

#include <restinio/all.hpp>
#include <taskflow/taskflow.hpp>

#include <dice/node-store/metall_manager.hpp>
#include <dice/triple-store/TripleStore.hpp>

namespace dice::endpoint {

	class SPARQLUpdateEndpoint {

		tf::Executor &executor_;

		triple_store::TripleStore &triplestore_;

	public:
		SPARQLUpdateEndpoint(tf::Executor &executor, triple_store::TripleStore &triplestore);

		restinio::request_handling_status_t operator()(
				restinio::request_handle_t req,
				restinio::router::route_params_t params);
	};

}// namespace dice::endpoint

#endif//TENTRIS_PRIVATE_SPARQLUPDATEENDPOINT_HPP
