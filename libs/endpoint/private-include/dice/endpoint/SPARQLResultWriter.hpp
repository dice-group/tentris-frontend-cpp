#ifndef TENTRIS_SPARQLRESULTWRITER_HPP
#define TENTRIS_SPARQLRESULTWRITER_HPP

#include <rdf4cpp/rdf.hpp>

#include <dice/rdf-tensor/Query.hpp>

namespace dice::endpoint {

	class SPARQLResultWriter {

	protected:
		std::size_t number_of_solutions_ = 0;
		std::size_t number_of_bindings_ = 0;

	public:
		virtual ~SPARQLResultWriter() = default;

		[[nodiscard]] virtual std::string ask_query_result(bool result) = 0;

		virtual void add(rdf_tensor::SolutionMapping const &solution_mapping) = 0;

		[[nodiscard]] virtual std::size_t number_of_written_solutions() const {
			return number_of_solutions_;
		}

		[[nodiscard]] virtual std::size_t number_of_written_bindings() const {
			return number_of_bindings_;
		}

		[[nodiscard]] virtual std::string_view string_view() = 0;

		[[nodiscard]] virtual std::string content_type() = 0;

		virtual void clear() = 0;

		virtual void close() = 0;

	};

}// namespace dice::endpoint

#endif//TENTRIS_SPARQLRESULTWRITER_HPP
