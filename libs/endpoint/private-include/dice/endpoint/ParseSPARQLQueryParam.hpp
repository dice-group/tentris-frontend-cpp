#ifndef TENTRIS_PARSESPARQLQUERYPARAM_HPP
#define TENTRIS_PARSESPARQLQUERYPARAM_HPP

#include <spdlog/spdlog.h>

#include <restinio/request_handler.hpp>
#include <restinio/uri_helpers.hpp>
#include <restinio/helpers/http_field_parsers/content-type.hpp>

#include <dice/sparql2tensor/SPARQLQuery.hpp>

#include <dice/endpoint/SparqlQueryCache.hpp>

namespace dice::endpoint {

	inline std::string parse_sparql_query_param(restinio::request_handle_t &req) {
		using namespace dice::sparql2tensor;
		using namespace restinio;
		// get request
		if (req->header().method() == http_method_get()) {
			const auto qp = parse_query<restinio::parse_query_traits::javascript_compatible>(req->header().query());
			if (not qp.has("query")) {
				static auto const message = "Query parameter 'query' is missing from GET request.";
				spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
				req->create_response(status_bad_request()).set_body(message).done();
				return {};
			}
			return std::string{qp["query"]};
		}
		// post request
		if (req->header().method() == http_method_post()) {
			auto content_type = req->header().opt_value_of(http_field::content_type);
			auto content_type_value = http_field_parsers::content_type_value_t::try_parse(*content_type);
			if (not content_type_value.has_value() or
				content_type_value.value().media_type.type != "application" or
				content_type_value.value().media_type.subtype != "sparql-query") {
				throw std::runtime_error("Expected content-type: application/sparql-query");
			}
			if (req->body().empty()) {
				static auto const message = "Body of POST request is empty; expecting SPARQL query";
				spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
				req->create_response(status_bad_request()).set_body(message).done();
				return {};
			}
			return req->body();
		}
	}
}// namespace dice::endpoint
#endif//TENTRIS_PARSESPARQLQUERYPARAM_HPP
