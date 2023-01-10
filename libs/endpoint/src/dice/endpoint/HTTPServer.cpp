#include "HTTPServer.hpp"

#include "dice/endpoint/SPARQLEndpoint.hpp"
#include "dice/endpoint/SPARQLStreamEndpoint.hpp"

#include <spdlog/spdlog.h>

namespace dice::endpoint {

	struct tentris_restinio_traits : public restinio::traits_t<
											 restinio::null_timer_manager_t,
											 restinio::null_logger_t,
											 restinio::router::express_router_t<>> {
		static constexpr bool use_connection_count_limiter = true;
	};

	HTTPServer::HTTPServer(tf::Executor &executor, triple_store::TripleStore &triplestore, EndpointCfg const &cfg)
		: executor_(executor),
		  triplestore_(triplestore),
		  router_(std::make_unique<restinio::router::express_router_t<>>()),
		  cfg_(cfg) {}

	void HTTPServer::operator()() {
		spdlog::info("Available endpoints:");
		router_->http_get(R"(/sparql)",
						  SPARQLEndpoint{executor_, triplestore_, cfg_.timeout_duration});
		spdlog::info("  GET /sparql?query= for SPARQL queries");

		router_->http_get(R"(/stream)",
						  SPARQLStreamEndpoint{executor_, triplestore_, cfg_.timeout_duration});
		spdlog::info("  GET /stream?query= for SPARQL SELECT queries with big result sets");

		router_->non_matched_request_handler(
				[](auto req) -> restinio::request_handling_status_t {
					return req->create_response(restinio::status_not_found()).connection_close().done();
				});

		spdlog::info("Use Ctrl+C on the terminal or SIGINT to shut down tentris gracefully. If tentris is killed or crashes, the index files will be corrupted.");
		restinio::run(
				restinio::on_thread_pool<tentris_restinio_traits>(cfg_.threads)
						.max_parallel_connections(cfg_.threads)
						.address("0.0.0.0")
						.port(cfg_.port)
						.request_handler(std::move(router_)));
	}
}// namespace dice::endpoint