#ifndef DICE_SPARQL_EXISTS_HPP
#define DICE_SPARQL_EXISTS_HPP

#include "PrimaryExpressions.hpp"
#include "Expression.hpp"

namespace dice::sparql2tensor::expressions {

	class Exists : public SPARQLExpression {
	private:
		std::vector<std::unique_ptr<PrimaryVarExpression>> vars_in_scope_;
		rdf_tensor::Query sub_query_;
		bool not_exists_;
	public:
		Exists(std::vector<std::unique_ptr<PrimaryVarExpression>> vars, rdf_tensor::Query sub_query, bool not_exists = false);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] Exists *clone_impl() const override;
	};

}// namespace dice::sparql2tensor::expressions

#endif//DICE_SPARQL_EXISTS_HPP
