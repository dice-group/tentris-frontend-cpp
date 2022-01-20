#include "SparqlEndpoint.hpp"

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

				const auto qp = restinio::parse_query(req->header().query());
				if (not qp.has("query"))
					return req->create_response(restinio::status_bad_request()).set_body("Query parameter 'query' is missing.").done();
				std::string sparql_query_str = std::string{qp["query"]};
				SPARQLQuery sparql_query;
				try {
					sparql_query = SPARQLQuery::parse(sparql_query_str);
				} catch (std::exception &ex) {
					return req->create_response(restinio::status_bad_request()).set_body("Failed to parse query.").done();
				}

				endpoint::SparqlJsonResultSAXWriter json_writer{sparql_query.projected_variables_, 100'000};

				for (auto const &entry : this->triplestore_.query(sparql_query, timeout)) {
					json_writer.add(entry);
				}
				return req->create_response(restinio::status_ok()).set_body(std::string{json_writer.string_view()}).done();
			},
								   std::move(req));
			return restinio::request_accepted();
		} else {
			return restinio::request_rejected();
		}
	}

}// namespace Dice::endpoint