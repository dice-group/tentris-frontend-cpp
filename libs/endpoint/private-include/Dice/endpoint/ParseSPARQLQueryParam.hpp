#ifndef TENTRIS_PARSESPARQLQUERYPARAM_HPP
#define TENTRIS_PARSESPARQLQUERYPARAM_HPP

#include <spdlog/spdlog.h>

#include <restinio/request_handler.hpp>
#include <restinio/uri_helpers.hpp>

#include <Dice/sparql2tensor/SPARQLQuery.hpp>

#include <Dice/endpoint/SparqlQueryCache.hpp>

namespace Dice::endpoint {

	inline std::string parse_sparql_query_param(restinio::request_handle_t &req) {
		using namespace Dice::sparql2tensor;
		using namespace restinio;
		const auto qp = parse_query(req->header().query());
		if (not qp.has("query")) {
			static auto const message = "Query parameter 'query' is missing.";
			spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
			req->create_response(status_bad_request()).set_body(message).done();
			return {};
		}
		return std::string{qp["query"]};
	}
}// namespace Dice::endpoint
#endif//TENTRIS_PARSESPARQLQUERYPARAM_HPP
