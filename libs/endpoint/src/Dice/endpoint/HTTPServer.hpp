#ifndef TENTRIS_HTTPSERVER_HPP
#define TENTRIS_HTTPSERVER_HPP

#include "SparqlEndpoint.hpp"
#include "SparqlStreamingEndpoint.hpp"
#include <restinio/all.hpp>
#include <taskflow/taskflow.hpp>


namespace Dice::endpoint {

	struct EndpointCfg {
		uint16_t port;
		uint16_t threads;
		std::chrono::seconds timeout_duration;
	};

	class HTTPServer {
		tf::Executor &executor_;
		triple_store::TripleStore &triplestore_;
		std::unique_ptr<restinio::router::express_router_t<>> router_;
		EndpointCfg cfg_;

	public:
		HTTPServer(tf::Executor &executor, triple_store::TripleStore &triplestore, EndpointCfg const &cfg);

		void operator()();
	};
}// namespace Dice::endpoint

#endif//TENTRIS_HTTPSERVER_HPP
