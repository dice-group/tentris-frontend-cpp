#ifndef TENTRIS_HTTPSERVER_HPP
#define TENTRIS_HTTPSERVER_HPP

#include <restinio/all.hpp>
#include <taskflow/taskflow.hpp>


namespace Dice::endpoint {

	struct EndpointCfg {
		uint16_t port;
		uint16_t threads;
	};

	class HTTPServer {
		struct tentris_restinio_traits : public restinio::traits_t<
												 restinio::null_timer_manager_t,
												 restinio::null_logger_t,
												 restinio::router::express_router_t<>> {
			static constexpr bool use_connection_count_limiter = true;
		};

		tf::Executor &executor_;
		std::unique_ptr<restinio::router::express_router_t<>> router_;

		EndpointCfg cfg_;


	public:
		HTTPServer(tf::Executor &executor, const EndpointCfg &cfg)
			: executor_(executor),
			  router_(std::make_unique<restinio::router::express_router_t<>>()),
			  cfg_(cfg) {
		}

		restinio::router::express_router_t<> &router() {
			return *router_;
		}

		EndpointCfg &config() {
			return cfg_;
		}


		void operator()() {
			router_->non_matched_request_handler(
			[](auto req) -> restinio::request_handling_status_t {
				return req->create_response(restinio::status_not_found()).connection_close().done();
			});
			restinio::run(
					restinio::on_thread_pool<tentris_restinio_traits>(cfg_.threads)
							.max_parallel_connections(cfg_.threads)
							.address("0.0.0.0")
							.port(cfg_.port)
							.request_handler(std::move(router_)));
		}
	};
}// namespace Dice::endpoint

#endif//TENTRIS_HTTPSERVER_HPP
