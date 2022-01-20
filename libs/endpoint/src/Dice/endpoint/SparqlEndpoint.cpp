#include "SparqlEndpoint.hpp"

#include "Dice/endpoint/HTTPHelper.hpp"
#include "Dice/endpoint/SparqlJsonResultSAXWriter.hpp"


namespace Dice::endpoint {

	SPARQLEndpoint::SPARQLEndpoint(tf::Executor &executor,
								   triple_store::TripleStore &triplestore,
								   std::chrono::seconds timeoutDuration)
		: executor_(executor),
		  triplestore_(triplestore),
		  timeout_duration_(timeoutDuration) {}

	restinio::request_handling_status_t SPARQLEndpoint::operator()(
			restinio::request_handle_t req,
			[[maybe_unused]] restinio::router::route_params_t params) {
		auto timeout = (timeout_duration_.count()) ? std::chrono::steady_clock::now() + this->timeout_duration_ : std::chrono::steady_clock::time_point::max();
		if (executor_.num_topologies() < executor_.num_workers()) {
			executor_.silent_async([this, timeout](restinio::request_handle_t req) {
				using namespace Dice::sparql2tensor;
				using namespace restinio;

				SPARQLQuery sparql_query;
				if (auto sparql_query_opt = get_sparql_query_param(req); sparql_query_opt.has_value())
					sparql_query = std::move(sparql_query_opt.value());
				else
					return;

				try {
					endpoint::SparqlJsonResultSAXWriter json_writer{sparql_query.projected_variables_, 100'000};

					for (auto const &entry : this->triplestore_.query(sparql_query, timeout)) {
						json_writer.add(entry);
					}

					req->create_response(status_ok())
							.append_header(http_field::content_type, "application/sparql-results+json")
							.set_body(std::string{json_writer.string_view()})
							.done();
				} catch (Dice::einsum::TimeoutException const &timeout_exception) {
					req->create_response(status_gateway_time_out()).done();
				}
			},
								   std::move(req));
			return restinio::request_accepted();
		} else {
			return restinio::request_rejected();
		}
	}

}// namespace Dice::endpoint