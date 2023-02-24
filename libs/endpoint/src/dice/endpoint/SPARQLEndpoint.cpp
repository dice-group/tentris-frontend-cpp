#include "SPARQLEndpoint.hpp"

#include <spdlog/spdlog.h>

#include <dice/endpoint/ParseSPARQLQueryParam.hpp>
#include <dice/endpoint/SparqlJsonResultSAXWriter.hpp>
#include <dice/endpoint/XMLResultWriter.hpp>

namespace dice::endpoint {

	SPARQLEndpoint::SPARQLEndpoint(tf::Executor &executor,
								   triplestore::TripleStore &triplestore,
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
				// parse request
				auto results_format = ResultFormat::JSON;
				auto sparql_query_str = parse_sparql_query_param(req, &results_format);
				if (sparql_query_str.empty())
					return;
				// replace new lines with single spaces (better logging)
				std::replace(sparql_query_str.begin(), sparql_query_str.end(), '\n', ' ');
				spdlog::info("SPARQL Query: {}", sparql_query_str);
				try {
					auto evaluation_context = triplestore_.create_evaluation_context(timeout);
					auto sparql_query = triplestore_.parse_sparql_query(sparql_query_str, evaluation_context);
					// instantiate writer
					std::unique_ptr<SPARQLResultWriter> result_writer;
					if (results_format == ResultFormat::JSON)
						result_writer = std::make_unique<SparqlJsonResultSAXWriter>(sparql_query->projected_variables(), 100'000);
					else if (results_format == ResultFormat::XML)
						result_writer = std::make_unique<XMLResultWriter>(sparql_query->projected_variables());
					else
						assert(false);
					// start the query evaluation
					auto result_generator = triplestore_.eval_sparql_query(*sparql_query, evaluation_context);
					if (sparql_query->query_type() == dice::sparql::SPARQLQuery::QueryType::ASK) {
						bool ask_res = result_generator.begin() != result_generator.end();
						req->create_response(status_ok())
								.append_header(http_field::content_type, result_writer->content_type())
								.set_body(result_writer->ask_query_result(ask_res))
								.done();
					} else {
						for (auto const &entry : result_generator) {
							result_writer->add(entry);
						}
						result_writer->close();

						req->create_response(status_ok())
								.append_header(http_field::content_type, result_writer->content_type())
								.set_body(std::string{result_writer->string_view()})
								.done();
						spdlog::info("HTTP response {}: {} variables, {} solutions, {} bindings",
									 status_ok(),
									 sparql_query->projected_variables().size(),
									 result_writer->number_of_written_solutions(),
									 result_writer->number_of_written_bindings());
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