#ifndef TENTRIS_PARSESPARQLUPDATEPARAM_HPP
#define TENTRIS_PARSESPARQLUPDATEPARAM_HPP

#include <spdlog/spdlog.h>

#include <restinio/request_handler.hpp>
#include <restinio/uri_helpers.hpp>
#include <restinio/helpers/http_field_parsers/content-type.hpp>

#include <dice/sparql2tensor/UPDATEQuery.hpp>


namespace dice::endpoint {

	inline sparql2tensor::UPDATEQuery parse_sparql_update_param(restinio::request_handle_t &req) {
		using namespace dice::sparql2tensor;
		using namespace restinio;
		auto content_type = req->header().opt_value_of(http_field::content_type);
		auto content_type_value = http_field_parsers::content_type_value_t::try_parse(*content_type);
		if (not content_type_value.has_value() or
			content_type_value.value().media_type.type != "application" or
			content_type_value.value().media_type.subtype != "sparql-update") {
			throw std::runtime_error("Expected content-type: application/sparql-update");
		}
		std::string sparql_update_str{req->body()};
		try {
			auto update_query = UPDATEQuery::parse(sparql_update_str);
			return update_query;
		} catch (std::exception &ex) {
			static constexpr auto message = "Value of parameter 'update' is not parsable: ";
			throw std::runtime_error{std::string{message} + ex.what()};
		} catch (...) {
			static constexpr auto message = "Unknown error";
			throw std::runtime_error{message};
		}
	}

	inline std::string extract_sparql_update_param(restinio::request_handle_t &req) {
		using namespace dice::sparql2tensor;
		using namespace restinio;
		auto content_type = req->header().opt_value_of(http_field::content_type);
		auto content_type_value = http_field_parsers::content_type_value_t::try_parse(*content_type);
		if (not content_type_value.has_value() or
			content_type_value.value().media_type.type != "application" or
			content_type_value.value().media_type.subtype != "sparql-update") {
			throw std::runtime_error("Expected content-type: application/sparql-update");
		}

		std::string_view sparql_delete_data_str = req->body();

		auto const is_whitespace = [](char c) { return std::isspace(c); };

		auto const delete_data_pos = sparql_delete_data_str.find("DELETE DATA");
		if (delete_data_pos == std::string::npos) {
			throw std::runtime_error{"Syntax error: no 'DELETE DATA' found but currently only DELETE DATA queries are supported"};
		}

		if (auto const before_delete_data = sparql_delete_data_str.substr(0, delete_data_pos);
			!std::ranges::all_of(before_delete_data, is_whitespace)) {

			throw std::runtime_error{"Syntax error: found content before 'DELETE DATA'. Hint: currently not supporting prefixes"};
		}

		auto const open_curly_pos = sparql_delete_data_str.find('{');

		if (open_curly_pos == std::string::npos) {
			throw std::runtime_error{"Syntax error: expected '{' after 'DELETE DATA'"};
		}

		auto const after_delete_data = delete_data_pos + std::string_view{"DELETE DATA"}.size();
		if (auto const between_delete_data_open_curly = sparql_delete_data_str.substr(after_delete_data, open_curly_pos - after_delete_data);
			!std::ranges::all_of(between_delete_data_open_curly, is_whitespace)) {

			throw std::runtime_error{"Syntax error: found content between 'DELETE DATA' and '{'"};
		}

		auto const close_curly_pos = sparql_delete_data_str.rfind('}');

		if (close_curly_pos == std::string::npos) {
			throw std::runtime_error{"Syntax error: expected '}' at end of query"};
		}

		if (auto const after_close_curly = std::string_view{sparql_delete_data_str}.substr(close_curly_pos + 1);
			!std::ranges::all_of(after_close_curly, is_whitespace)) {

			throw std::runtime_error{"Syntax error: found content after last '}'"};
		}

		return std::string{sparql_delete_data_str.substr(open_curly_pos + 1, close_curly_pos - open_curly_pos)};
	}
}// namespace dice::endpoint


#endif//TENTRIS_PARSESPARQLUPDATEPARAM_HPP
