#include "SparqlUpdateEndpoint.hpp"

#include "dice/endpoint/ParseSPARQLUpdateParam.hpp"

#include <rapidjson/stringbuffer.h>

#include <rapidjson/writer.h>
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

					spdlog::debug("to remove size: {}", update_query.entries_for_removal.size());
					spdlog::debug("hypertrie size: {}", triplestore_.size());

					size_t const size_before = triplestore_.size();
					triplestore_.remove(update_query.entries_for_removal);
					size_t const mutation_count = size_before - triplestore_.size();

					rapidjson::StringBuffer buf;
					{
						rapidjson::Writer<rapidjson::StringBuffer> jw{buf};

						jw.StartObject();
						jw.Key("mutation_count");
						jw.Uint64(mutation_count);
						jw.EndObject();
					}

					req->create_response(status_ok())
							.append_header(http_field::content_type, "application/json")
							.set_body(std::string{buf.GetString(), buf.GetSize()})
							.done();

					spdlog::debug("hypertrie size after: {}", triplestore_.size());
					spdlog::info("HTTP response {}, mutation_count: {}", status_ok(), mutation_count);
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