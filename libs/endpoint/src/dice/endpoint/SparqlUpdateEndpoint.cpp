#include "SparqlUpdateEndpoint.hpp"

#include "dice/endpoint/ParseSPARQLUpdateParam.hpp"

#include <spdlog/spdlog.h>

namespace dice::endpoint {

	SPARQLUpdateEndpoint::SPARQLUpdateEndpoint(tf::Executor &executor,
											   triple_store::TripleStore &triplestore)
		: executor_(executor),
		  triplestore_(triplestore) {}

	restinio::request_handling_status_t SPARQLUpdateEndpoint::operator()(
			restinio::request_handle_t req,
			[[maybe_unused]] restinio::router::route_params_t params) {
		if (executor_.num_topologies() < executor_.num_workers()) {
			executor_.silent_async([this](restinio::request_handle_t req) {
				using namespace dice::sparql2tensor;
				using namespace restinio;

				try {
					auto const update_query = parse_sparql_update_param(req);

					for (auto const &entry : update_query.entries_for_removal) {
						spdlog::debug("removing triple ({}, {}, {})", entry[0], entry[1], entry[2]);
					}

					triplestore_.remove(update_query.entries_for_removal);

					req->create_response(status_ok()).done();
					spdlog::info("HTTP response {}", status_ok());
				} catch (std::runtime_error const &e) {
					static constexpr auto message = "Invalid Content-Type";

					req->create_response(status_bad_request()).set_body(message).done();
					spdlog::warn("HTTP response {}: {} (detail: {})", status_bad_request(), message, e.what());
				}
			}, std::move(req));

			return restinio::request_accepted();
		} else {
			spdlog::warn("Handling request was rejected. All workers are busy.");
			return restinio::request_rejected();
		}
	}

}// namespace dice::endpoint