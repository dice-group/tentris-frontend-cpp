#ifndef TENTRIS_PARSESPARQLQUERYPARAM_HPP
#define TENTRIS_PARSESPARQLQUERYPARAM_HPP

#include <spdlog/spdlog.h>

#include <restinio/helpers/http_field_parsers/accept.hpp>
#include <restinio/helpers/http_field_parsers/content-type.hpp>
#include <restinio/helpers/http_field_parsers/try_parse_field.hpp>
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
		using namespace restinio::http_field_parsers;
		// no idea why try_parse_field<accept_value_t> does not compile
		if (format != nullptr) {
			auto accept_header = req->header().opt_value_of(http_field::accept);
			if (accept_header.has_value()) {
				const auto parsed = accept_value_t::try_parse(*accept_header);
				if (parsed.has_value()) {
					uint32_t selected_priority = 0;
					for (const auto &v : parsed->items) {
						if (v.media_type.type != "application")
							continue;
						if (v.media_type.subtype == "sparql-results+json") {
							uint32_t p = v.weight.has_value() ? v.weight->as_uint() : 1000;
							if (p > selected_priority) {
								*format = ResultFormat::JSON;
								selected_priority = p;
							}
						} else if (v.media_type.subtype == "sparql-results+xml") {
							uint32_t p = v.weight.has_value() ? v.weight->as_uint() : 1000;
							if (p > selected_priority) {
								*format = ResultFormat::XML;
								selected_priority = p;
							}
						}
					}
					if (selected_priority == 0)
						*format = ResultFormat::JSON;
				} else {
					static auto const message = "The requested result format is invalid";
					spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
					req->create_response(status_bad_request()).set_body(message).done();
					return {};
				}
			}
		}
		// get request
		if (req->header().method() == http_method_get()) {
			try {
				const auto qp = parse_query<restinio::parse_query_traits::javascript_compatible>(req->header().query());
				if (not qp.has("query")) {
					static auto const message = "Query parameter 'query' is missing from GET request.";
					spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
					req->create_response(status_bad_request()).set_body(message).done();
					return {};
				}
				return std::string{qp["query"]};
			} catch (std::exception &e) {
				static auto const message = e.what();
				spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
				req->create_response(status_bad_request()).set_body(message).done();
				return {};
			}
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
		try {
			const auto parsed_body = restinio::parse_query<restinio::parse_query_traits::x_www_form_urlencoded>(req->body());
			if (not parsed_body.has("query")) {
				static auto const message = "Query parameter 'query' is missing from POST (application/x-www-form-urlencoded) request.";
				spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
				req->create_response(status_bad_request()).set_body(message).done();
				return {};
			}
			return std::string{parsed_body["query"]};
		} catch (std::exception &e) {
			static auto const message = e.what();
			spdlog::warn("HTTP response {}: {}", status_bad_request(), message);
			req->create_response(status_bad_request()).set_body(message).done();
			return {};
		}
	}
}// namespace dice::endpoint
#endif//TENTRIS_PARSESPARQLQUERYPARAM_HPP
