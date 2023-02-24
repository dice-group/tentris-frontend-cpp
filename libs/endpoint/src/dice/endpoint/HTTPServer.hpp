#ifndef TENTRIS_HTTPSERVER_HPP
#define TENTRIS_HTTPSERVER_HPP

#include <restinio/all.hpp>
#include <taskflow/taskflow.hpp>

#include <dice/tentris/param_allocator.hpp>
#include <dice/triplestore/TripleStore.hpp>


namespace dice::endpoint {

	struct EndpointCfg {
		uint16_t port;
		uint16_t threads;
		std::chrono::seconds timeout_duration;
	};

	class HTTPServer {
		tf::Executor &executor_;
		triplestore::TripleStore &triplestore_;
		std::unique_ptr<restinio::router::express_router_t<>> router_;
		EndpointCfg cfg_;

	public:
		HTTPServer(tf::Executor &executor, triplestore::TripleStore &triplestore, EndpointCfg const &cfg);

		restinio::router::express_router_t<> &router(){
			return *router_;
		}

		void operator()();
	};
}// namespace dice::endpoint

#endif//TENTRIS_HTTPSERVER_HPP
