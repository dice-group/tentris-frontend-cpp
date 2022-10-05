#ifndef DICE_SPARQL_EXISTS_HPP
#define DICE_SPARQL_EXISTS_HPP

#include "Expression.hpp"

namespace dice::sparql2tensor::expressions {

	class Exists : public SPARQLExpression {
	private:
		rdf_tensor::Query sub_query_;
		bool not_exists_;
		std::chrono::steady_clock::time_point timeout_;
		std::vector<rdf4cpp::rdf::query::Variable> variables_;
		boost::container::flat_map<char, size_t> var_ids_positions_;
		rdf4cpp::rdf::Literal true_{"true", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
		rdf4cpp::rdf::Literal false_{"false", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};

	public:
		Exists(std::vector<rdf4cpp::rdf::query::Variable> variables,
			   boost::container::flat_map<char, size_t> var_ids_positions,
			   rdf_tensor::Query sub_query,
			   bool not_exists,
			   std::chrono::steady_clock::time_point timeout);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;

	protected:
		[[nodiscard]] Exists *clone_impl() const override;
	};

}// namespace dice::sparql2tensor::expressions

#endif//DICE_SPARQL_EXISTS_HPP
