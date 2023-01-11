#ifndef TENTRIS_PARSESPARQLQUERYPARAM_HPP
#define TENTRIS_PARSESPARQLQUERYPARAM_HPP

#include <spdlog/spdlog.h>

#include <restinio/helpers/http_field_parsers/content-type.hpp>
#include <restinio/request_handler.hpp>
#include <restinio/uri_helpers.hpp>

namespace dice::endpoint {

	enum ResultFormat {
		JSON = 0,
		XML,
		CSV,
		TSV
	};

	inline std::string parse_sparql_query_param(restinio::request_handle_t &req, ResultFormat *format = nullptr) {
		using namespace restinio;
		if (format != nullptr) {
			auto accept_header = req->header().opt_value_of(http_field::accept);
			if (accept_header.has_value()) {
				auto const &accept_header_value = accept_header.value();
				std::cout << accept_header_value << std::endl;
				// default (*/*) to JSON
				if (accept_header_value == "*/*" or accept_header_value == "application/sparql-results+json")
					*format = ResultFormat::JSON;
				else if (accept_header_value == "application/sparql-results+xml")
					*format = ResultFormat::XML;
				else
					throw std::runtime_error("The requested result format is not supported. Currently only XML and JSON formats are supported");
			}
		}
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
		auto content_type = req->header().opt_value_of(http_field::content_type);
		auto content_type_value = http_field_parsers::content_type_value_t::try_parse(*content_type);
		if (not content_type_value.has_value() or
			content_type_value.value().media_type.type != "application" or
			(content_type_value.value().media_type.subtype != "sparql-query" and
			 content_type_value.value().media_type.subtype != "x-www-form-urlencoded")) {
			static auto const message = "Expected content-type: application/sparql-query or application/x-www-form-urlencoded";
			spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
			req->create_response(status_bad_request()).set_body(message).done();
			return {};
		}
		// post application/sparql-query
		if (content_type_value.value().media_type.subtype == "sparql-query") {
			if (req->body().empty()) {
				static auto const message = "Body of POST request with content-type application/sparql-query is empty; "
											"expecting unencoded SPARQL query";
				spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
				req->create_response(status_bad_request()).set_body(message).done();
				return {};
			}
			return req->body();
		}
		// post application/x-www-form-urlencoded
		const auto parsed_body = restinio::parse_query<restinio::parse_query_traits::x_www_form_urlencoded>(req->body());
		if (not parsed_body.has("query")) {
			static auto const message = "Query parameter 'query' is missing from POST (application/x-www-form-urlencoded) request.";
			spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
			req->create_response(status_bad_request()).set_body(message).done();
			return {};
		}
		return std::string{parsed_body["query"]};
	}
}// namespace dice::endpoint
#endif//TENTRIS_PARSESPARQLQUERYPARAM_HPP
