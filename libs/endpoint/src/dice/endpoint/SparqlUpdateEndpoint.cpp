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

				// update query and parsing
				auto update_query = parse_sparql_update_param(req);
				for (auto const &entry : update_query.entries_for_removal) {
					std::cout << entry[0] << " " << entry[1] << " " << entry[2] << std::endl;
				}
				// todo: call hypertrie function for removal and construct response
			},
								   std::move(req));
			return restinio::request_accepted();
		} else {
			spdlog::warn("Handling request was rejected. All workers are busy.");
			return restinio::request_rejected();
		}
	}

}// namespace dice::endpoint