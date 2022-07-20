#ifndef DICE_SPARQL_EXPRESSION_HPP
#define DICE_SPARQL_EXPRESSION_HPP

#include <Dice/rdf_tensor/Query.hpp>
#include <rdf4cpp/rdf.hpp>

namespace Dice::sparql2tensor::expressions {

	class SPARQLExpression : public rdf_tensor::Expression {
	public:
		SPARQLExpression() = default;
		[[nodiscard]] std::unique_ptr<SPARQLExpression> clone() const { return std::unique_ptr<SPARQLExpression>(clone_impl()); }
		[[nodiscard]] virtual std::vector<rdf4cpp::rdf::query::Variable> variables() const = 0;
	protected:
		[[nodiscard]] virtual SPARQLExpression *clone_impl() const = 0;
	};

}//namespace Dice::sparql2tensor::expressions

#endif//DICE_SPARQL_EXPRESSION_HPP
