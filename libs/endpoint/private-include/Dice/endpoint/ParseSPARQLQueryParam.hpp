#ifndef TENTRIS_PARSESPARQLQUERYPARAM_HPP
#define TENTRIS_PARSESPARQLQUERYPARAM_HPP

#include <spdlog/spdlog.h>

#include <restinio/request_handler.hpp>
#include <restinio/uri_helpers.hpp>

#include <Dice/sparql2tensor/SPARQLQuery.hpp>

#include <Dice/endpoint/SparqlQueryCache.hpp>

namespace Dice::endpoint {

	inline std::shared_ptr<sparql2tensor::SPARQLQuery const> parse_sparql_query_param(restinio::request_handle_t &req, SparqlQueryCache &cache) {
		using namespace Dice::sparql2tensor;
		using namespace restinio;
		const auto qp = parse_query(req->header().query());
		if (not qp.has("query")) {
			static auto const message = "Query parameter 'query' is missing.";
			spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
			req->create_response(status_bad_request()).set_body(message).done();
			return {};
		}
		std::string sparql_query_str = std::string{qp["query"]};
		SPARQLQuery sparql_query;
		try {
			return cache[sparql_query_str];
		} catch (std::exception &ex) {
			static auto const message = "Value of query parameter 'query' is not parsable.";
			spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
			req->create_response(status_bad_request()).set_body(message).done();
			return {};
		}
	}
}// namespace Dice::endpoint
#endif//TENTRIS_PARSESPARQLQUERYPARAM_HPP
