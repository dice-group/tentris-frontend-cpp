#ifndef TENTRIS_HTTPSERVER_HPP
#define TENTRIS_HTTPSERVER_HPP

#include <restinio/all.hpp>
#include <taskflow/taskflow.hpp>

#include <Dice/triple_store/TripleStore.hpp>

#include <Dice/endpoint/SparqlQueryCache.hpp>


namespace Dice::endpoint {

	struct EndpointCfg {
		uint16_t port;
		uint16_t threads;
		std::chrono::seconds timeout_duration;
	};

	class HTTPServer {
		tf::Executor &executor_;
		triple_store::TripleStore &triplestore_;
		SparqlQueryCache sparql_query_cache_;
		std::unique_ptr<restinio::router::express_router_t<>> router_;
		EndpointCfg cfg_;

	public:
		HTTPServer(tf::Executor &executor, triple_store::TripleStore &triplestore, EndpointCfg const &cfg);

		void operator()();
	};
}// namespace Dice::endpoint

#endif//TENTRIS_HTTPSERVER_HPP
