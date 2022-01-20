#ifndef TENTRIS_HTTPHELPER_HPP
#define TENTRIS_HTTPHELPER_HPP

#include <restinio/request_handler.hpp>
#include <restinio/uri_helpers.hpp>

#include <Dice/sparql2tensor/SPARQLQuery.hpp>

namespace Dice::endpoint {

	inline std::optional<sparql2tensor::SPARQLQuery> get_sparql_query_param(restinio::request_handle_t &req) {
		using namespace Dice::sparql2tensor;
		using namespace restinio;
		const auto qp = parse_query(req->header().query());
		if (not qp.has("query")) {
			req->create_response(status_bad_request()).set_body("Query parameter 'query' is missing.").done();
			return std::nullopt;
		}
		std::string sparql_query_str = std::string{qp["query"]};
		SPARQLQuery sparql_query;
		try {
			sparql_query = SPARQLQuery::parse(sparql_query_str);
		} catch (std::exception &ex) {
			req->create_response(status_bad_request()).set_body("Failed to parse query.").done();
			return std::nullopt;
		}
		return sparql_query;
	}
}// namespace Dice::endpoint
#endif//TENTRIS_HTTPHELPER_HPP
