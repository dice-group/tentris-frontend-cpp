#ifndef TENTRIS_XMLRESULTWRITER_HPP
#define TENTRIS_XMLRESULTWRITER_HPP

#include "SPARQLResultWriter.hpp"

#include <pugixml.hpp>

namespace dice::endpoint {

	class XMLResultWriter : public SPARQLResultWriter {
		using Node = rdf4cpp::rdf::Node;
		using Literal = rdf4cpp::rdf::Literal;
		using IRI = rdf4cpp::rdf::IRI;
		using BlankNode = rdf4cpp::rdf::BlankNode;
		using Variable = rdf4cpp::rdf::query::Variable;
		using SolutionMapping = dice::rdf_tensor::SolutionMapping;

		std::vector<std::string> variables_;
		pugi::xml_document xml_results_;
		pugi::xml_node result_section_;
		std::string xml_results_str_;

	public:
		explicit XMLResultWriter(const std::vector<Variable>& variables) {
			auto sparql_document_element = xml_results_.append_child("sparql");
			sparql_document_element.append_attribute("xmlns").set_value("http://www.w3.org/2005/sparql-results#");
			auto head_section = sparql_document_element.append_child("head");
			for (auto const &var : variables) {
				auto const &var_name = std::string(var.name());
				head_section.append_child("variable").append_attribute("name").set_value(var_name.c_str());
				variables_.push_back(var_name);
			}
			result_section_ = sparql_document_element.append_child("results");
		}

		[[nodiscard]] std::string ask_query_result(bool result) override {
			std::string ask_res_str = result ? "true" : "false";
			return "<?xml version=\"1.0\"?>\n"
				   "<sparql xmlns=\"http://www.w3.org/2005/sparql-results#\">\n"
				   "  <head></head>\n"
				   "  <boolean>"+ask_res_str+"</boolean>\n"
				   "</sparql>";
		}

		void close() override {
			std::stringstream str_stream;
			xml_results_.save(str_stream);
			xml_results_str_ = str_stream.str();
			xml_results_.reset(); // clear
		}

		void add(SolutionMapping const &solution_mapping) override {
			for (size_t i = 0; i < size_t(solution_mapping.value()); ++i) {
				auto result = result_section_.append_child("result");
				for (const auto &[term, var] : iter::zip(solution_mapping.key(), variables_)) {
					if (term.null())
						continue;
					auto binding = result.append_child("binding");
					binding.append_attribute("name").set_value(var.c_str());
					if (term.is_iri()) {
						auto const &identifier = ((IRI) term).identifier();
						binding.append_child("uri").append_child(pugi::node_pcdata).set_value(identifier.data(), identifier.size());
					} else if (term.is_literal()) {
						auto literal_node = binding.append_child("literal");
						auto literal = (Literal) term;
						static const IRI xsd_str{"http://www.w3.org/2001/XMLSchema#string"};
						auto datatype = literal.datatype();
						if (datatype != xsd_str) {
							auto const &lang = literal.language_tag();
							if (not lang.empty()) {
								literal_node.append_attribute("xml:lang").set_value(lang.data(), lang.size());
							} else {
								literal_node.append_attribute("datatype").set_value(datatype.identifier().data(), datatype.identifier().size());
							}
						}
						auto const &lexical_form = literal.lexical_form().view();
						literal_node.append_child(pugi::node_pcdata).set_value(lexical_form.data(), lexical_form.size());
					} else if (term.is_blank_node()) {
						auto const &identifier = ((BlankNode) term).identifier();
						binding.append_child("bnode").append_child(pugi::node_pcdata).set_value(identifier.data(), identifier.size());
					} else {
						throw std::runtime_error("Node with incorrect type (none of Literal, BNode, URI) detected.");
					}
					number_of_bindings_++;
				}
			}
			number_of_solutions_ += solution_mapping.value();
		}

		[[nodiscard]] std::string_view string_view() override {
			return {xml_results_str_};
		}

		[[nodiscard]] std::string content_type() override {
			return "application/sparql-results+xml";
		}

		void clear() override {
			xml_results_.reset();
		}

	};

}// namespace dice::endpoint

#endif//TENTRIS_XMLRESULTWRITER_HPP
