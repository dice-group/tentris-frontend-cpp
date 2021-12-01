#ifndef TENTRIS_SPARQLJSONRESULTSAXWRITER_HPP
#define TENTRIS_SPARQLJSONRESULTSAXWRITER_HPP

#include <itertools.hpp>
#include <rdf4cpp/rdf.hpp>
#include <utility>

#include "tentris/store/RDF/TermStore.hpp"
#include "tentris/util/LogHelper.hpp"

#define RAPIDJSON_HAS_STDSTRING 1

#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/pointer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "tentris/tensor/BoolHypertrie.hpp"

namespace tentris::store {

	template<typename result_type>
	class SparqlJsonResultSAXWriter {
		using Term = rdf4cpp::rdf::Node;
		using Literal = rdf4cpp::rdf::Literal;
		using Variable = rdf4cpp::rdf::query::Variable;
		using Solution = ::tentris::tensor::Solution<result_type>;

		std::size_t result_count = 0;
		std::size_t term_count_ = 0;

		std::vector<Variable> variables{};

		size_t buffer_size;
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer;


	public:
		explicit SparqlJsonResultSAXWriter(std::vector<Variable> variables, size_t buffer_size)
			: variables(std::move(variables)),
			  buffer_size(buffer_size),
			  buffer(nullptr, size_t(buffer_size * 1.3)),
			  writer(buffer) {
			writer.StartObject();
			writer.Key("head");
			{
				writer.StartObject();
				writer.Key("vars");
				{
					writer.StartArray();
					for (const auto &var : this->variables)
						writer.String(var.name());
					writer.EndArray();
				}
				writer.EndObject();
			}
			writer.Key("results");
			writer.StartObject();
			writer.Key("bindings");
			writer.StartArray();
		}

		void close() {
			writer.EndArray();
			writer.EndObject();
			writer.EndObject();
		}

		void add(const Solution &solution) {

			for (size_t i = 0; i < size_t(solution.value()); ++i) {
				writer.StartObject();
				for (const auto &[term, var] : iter::zip(solution.key(), variables)) {
					if (term == nullptr)
						continue;
					writer.Key(var.name());
					writer.StartObject();
					writer.Key("type");
					if (term->is_iri())
						writer.String("uri");
					else if (term->is_blank_node())
						writer.String("bnode");
					else if (term->is_literal())
						writer.String("literal");
					else {
						logging::log("Incomplete term with no type (Literal, BNode, URI) detected.");
						assert(false);
					}
					writer.Key("value");

					auto value = std::string(*term);
					writer.String(value.data(), value.size());

					if (term->is_literal()) {
						auto literal_term = rdf4cpp::rdf::Literal(*term);
						if (not literal_term.datatype().null()) {
							auto data_type = literal_term.datatype().identifier();
							writer.Key("datatype");
							writer.String(data_type.data(), data_type.size());
						} else if (not literal_term.language_tag().empty()) {
							auto lang = literal_term.language_tag();
							writer.Key("xml:lang");
							writer.String(lang.data(), lang.size());
						}
					}
					writer.EndObject();
					term_count_++;
				}
			}
			writer.EndObject();

			result_count += solution.value();
		}

		[[nodiscard]] std::size_t
		size() const {
			return buffer.GetSize();
		}

		[[nodiscard]] bool full() const {
			return buffer.GetSize() > this->buffer_size;
		};

		std::string_view string_view() {
			writer.Flush();
			return std::string_view(buffer.GetString(), buffer.GetSize());
		}

		void clear() {
			this->buffer.Clear();
		}
	};
}// namespace tentris::store

#endif//TENTRIS_SPARQLJSONRESULTSAXWRITER_HPP
