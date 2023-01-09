#ifndef TENTRIS_PARSESPARQLQUERYPARAM_HPP
#define TENTRIS_PARSESPARQLQUERYPARAM_HPP

#include <spdlog/spdlog.h>

#include <restinio/request_handler.hpp>
#include <restinio/uri_helpers.hpp>

namespace dice::endpoint {

	inline std::string parse_sparql_query_param(restinio::request_handle_t &req) {
		using namespace restinio;
		const auto qp = parse_query<restinio::parse_query_traits::javascript_compatible>(req->header().query());
		if (not qp.has("query")) {
			static auto const message = "Query parameter 'query' is missing.";
			spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
			req->create_response(status_bad_request()).set_body(message).done();
			return {};
		}
		return std::string{qp["query"]};
	}
}// namespace dice::endpoint
#endif//TENTRIS_PARSESPARQLQUERYPARAM_HPP
