#include "SparqlEndpoint.hpp"

#include <spdlog/spdlog.h>

#include "Dice/endpoint/ParseSPARQLQueryParam.hpp"
#include "Dice/endpoint/SparqlJsonResultSAXWriter.hpp"

#include <Dice/sparql2tensor/parser/SPARQLParser.hpp>

namespace Dice::endpoint {

	SPARQLEndpoint::SPARQLEndpoint(tf::Executor &executor,
								   triple_store::TripleStore &triplestore,
								   SparqlQueryCache &sparql_query_cache,
								   std::chrono::seconds timeoutDuration)
		: executor_(executor),
		  triplestore_(triplestore),
		  sparql_query_cache_(sparql_query_cache),
		  timeout_duration_(timeoutDuration) {}

	restinio::request_handling_status_t SPARQLEndpoint::operator()(
			restinio::request_handle_t req,
			[[maybe_unused]] restinio::router::route_params_t params) {
		auto timeout = (timeout_duration_.count()) ? std::chrono::steady_clock::now() + this->timeout_duration_ 
												   : std::chrono::steady_clock::time_point::max();
		if (executor_.num_topologies() < executor_.num_workers()) {
			executor_.silent_async([this, timeout](restinio::request_handle_t req) {
				using namespace Dice::sparql2tensor;
				using namespace restinio;
				// parse request
				std::string sparql_query_str = parse_sparql_query_param(req);
				if (sparql_query_str.empty())
					return;
				// parse query
				SPARQLQuery sparql_query = parser::SPARQLParser::parse_query(sparql_query_str, triplestore_);
				try {
					if (sparql_query.ask()) {
						auto generator_iter = rdf_tensor::QueryEvaluation::evaluate(sparql_query.raw_query(), timeout);
						bool ask_res = false;
						if (generator_iter.begin() != generator_iter.end() and (*generator_iter.begin()).value() > 0)
							ask_res = true;
						std::string ask_res_str = ask_res ? "true" : "false";
						req->create_response(status_ok())
								.append_header(http_field::content_type, "application/sparql-results+json")
								.set_body(R"({ "head" : {}, "boolean" : )" + ask_res_str + " }")
								.done();
					} else {
						endpoint::SparqlJsonResultSAXWriter json_writer{sparql_query.projected_variables(), 100'000};

						for (auto const &entry : rdf_tensor::QueryEvaluation::evaluate(sparql_query.raw_query(), timeout)) {
							json_writer.add(entry);
						}
						json_writer.close();

						req->create_response(status_ok())
								.append_header(http_field::content_type, "application/sparql-results+json")
								.set_body(std::string{json_writer.string_view()})
								.done();
						spdlog::info("HTTP response {}: {} variables, {} solutions, {} bindings",
									 status_ok(),
									 sparql_query.projected_variables().size(),
									 json_writer.number_of_written_solutions(),
									 json_writer.number_of_written_bindings());
					}
				} catch (std::runtime_error const &timeout_exception) {
					const auto timeout_message = fmt::format("Request processing timed out after {}.", this->timeout_duration_);
					spdlog::warn("HTTP response {}: {}", status_gateway_time_out(), timeout_message);
					req->create_response(status_gateway_time_out()).set_body(timeout_message).done();
				}
			},
								   std::move(req));
			return restinio::request_accepted();
		} else {
			spdlog::warn("Handling request was rejected. All workers are busy.");
			return restinio::request_rejected();
		}
	}

}// namespace Dice::endpoint