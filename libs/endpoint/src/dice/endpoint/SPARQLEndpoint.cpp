#include "SPARQLEndpoint.hpp"

#include <spdlog/spdlog.h>

#include <dice/endpoint/ParseSPARQLQueryParam.hpp>
#include <dice/endpoint/SparqlJsonResultSAXWriter.hpp>
#include <dice/endpoint/XMLResultWriter.hpp>

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
				// replace new lines with single spaces (better logging)
				std::replace(sparql_query_str.begin(), sparql_query_str.end(), '\n', ' ');
				spdlog::info("SPARQL Query: {}", sparql_query_str);
				try {
					auto sparql_query = triplestore_.parse_sparql_query(sparql_query_str, timeout);
					auto result_generator = triplestore_.eval_sparql_query(*sparql_query, timeout);
					if (sparql_query->ask()) {
						bool ask_res = result_generator.begin() != result_generator.end();
						std::string ask_res_str = ask_res ? "true" : "false";
						req->create_response(status_ok())
								.append_header(http_field::content_type, "application/sparql-results+json")
								.set_body(R"({ "head" : {}, "boolean" : )" + ask_res_str + " }")
								.done();
					} else {
						std::unique_ptr<SPARQLResultWriter> json_writer = std::make_unique<XMLResultWriter>(sparql_query->projected_variables());
						auto raw_query = sparql_query->raw_query();
						for (auto const &entry : result_generator) {
							json_writer->add(entry);
						}
						json_writer->close();

						req->create_response(status_ok())
								.append_header(http_field::content_type, "application/sparql-results+json")
								.set_body(std::string{json_writer->string_view()})
								.done();
						spdlog::info("HTTP response {}: {} variables, {} solutions, {} bindings",
									 status_ok(),
									 sparql_query->projected_variables().size(),
									 json_writer->number_of_written_solutions(),
									 json_writer->number_of_written_bindings());
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