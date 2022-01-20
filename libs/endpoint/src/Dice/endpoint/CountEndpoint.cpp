#include "CountEndpoint.hpp"

#include <spdlog/spdlog.h>

#include "Dice/endpoint/HTTPHelper.hpp"
#include "Dice/endpoint/SparqlJsonResultSAXWriter.hpp"

namespace Dice::endpoint {
	CountEndpoint::CountEndpoint(tf::Executor &executor,
								 triple_store::TripleStore &triplestore,
								 std::chrono::seconds timeoutDuration)
		: executor_(executor),
		  triplestore_(triplestore),
		  timeout_duration_(timeoutDuration) {}

	restinio::request_handling_status_t CountEndpoint::operator()(
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
					size_t count = 0;
					if (sparql_query.triple_patterns_.size() == 1) { // O(1)
						Dice::hypertrie::SliceKey<tr> slice_key = sparql_query.get_slice_keys()[0];
						if (slice_key.get_fixed_depth() == 3)
							count = (size_t) std::get<bool>(triplestore_.get_hypertrie()[slice_key]);
						else
							count = std::get<sparql2tensor::const_BoolHypertrie>(triplestore_.get_hypertrie()[slice_key]).size();
					} else
						for (auto const &entry : this->triplestore_.query(sparql_query, timeout))
							count += entry.value();

					req->create_response(status_ok())
							.set_body(fmt::format("{}", count))
							.done();
					spdlog::info("HTTP response {}: counted {} results", status_ok(), count);
				} catch (Dice::einsum::TimeoutException const &timeout_exception) {
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