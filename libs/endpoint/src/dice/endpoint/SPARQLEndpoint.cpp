#include "SPARQLEndpoint.hpp"

#include <spdlog/spdlog.h>

#include "dice/endpoint/ParseSPARQLQueryParam.hpp"
#include "dice/endpoint/SparqlJsonResultSAXWriter.hpp"

namespace dice::endpoint {

	SPARQLEndpoint::SPARQLEndpoint(tf::Executor &executor,
								   triple_store::TripleStore &triplestore,
								   std::chrono::seconds timeoutDuration)
		: executor_(executor),
		  triplestore_(triplestore),
		  timeout_duration_(timeoutDuration) {}

	restinio::request_handling_status_t SPARQLEndpoint::operator()(
			restinio::request_handle_t const &req,
			[[maybe_unused]] restinio::router::route_params_t params) {
		auto timeout = (timeout_duration_.count()) ? std::chrono::steady_clock::now() + this->timeout_duration_ : std::chrono::steady_clock::time_point::max();
		if (executor_.num_topologies() < executor_.num_workers()) {
			executor_.silent_async([this, timeout](restinio::request_handle_t req) {
				using namespace restinio;

				auto sparql_query_str = parse_sparql_query_param(req);
				if (sparql_query_str.empty())
					return;

				try {
					auto query_res = triplestore_.eval_query(sparql_query_str, timeout);
					if (std::holds_alternative<bool>(query_res)) {
						bool ask_res = std::get<0>(query_res);
						std::string res = ask_res ? "true" : "false";
						req->create_response(status_ok())
								.append_header(http_field::content_type, "application/sparql-results+json")
								.set_body(R"({ "head" : {}, "boolean" : )" + res + " }")
								.done();
					} else {
						auto &[projected_variables, solution_mappings] = std::get<1>(query_res);
						endpoint::SparqlJsonResultSAXWriter json_writer{projected_variables, 100'000};

						bool asio_write_failed = false;
						response_builder_t<chunked_output_t> resp = req->template create_response<chunked_output_t>();
						resp.append_header(http_field::content_type, "application/sparql-results+json");

						for (auto const &entry : solution_mappings) {
							json_writer.add(entry);
							if (json_writer.full()) {
								resp.append_chunk(std::string{json_writer.string_view()});
								resp.flush([&](auto const &status) { asio_write_failed = status.failed(); });
								if (asio_write_failed) {
									spdlog::warn("Writing chunked HTTP response failed.");
									return;
								}
								json_writer.clear();
							}
						}
						json_writer.close();
						resp.append_chunk(std::string{json_writer.string_view()});
						resp.done();

						spdlog::info("HTTP response {}: {} variables, {} solutions, {} bindings",
									 status_ok(),
									 projected_variables.size(),
									 json_writer.number_of_written_solutions(),
									 json_writer.number_of_written_bindings());
					}
				}
				// todo: different types of exceptions (e.g., parsing exception and timeout exception)
				catch (std::runtime_error const &runtime_exception) {
					const auto message = fmt::format("Error: {}", runtime_exception.what());
					spdlog::warn("HTTP response {}: {}", status_gateway_time_out(), message);
					req->create_response(status_gateway_time_out()).set_body(message).done();
				}
			},
								   std::move(req));
			return restinio::request_accepted();
		} else {
			spdlog::warn("Handling request was rejected. All workers are busy.");
			return restinio::request_rejected();
		}
	}

}// namespace dice::endpoint