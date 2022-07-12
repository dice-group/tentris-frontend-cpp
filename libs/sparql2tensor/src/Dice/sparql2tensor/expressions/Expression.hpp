#ifndef DICE_SPARQL_EXPRESSION_HPP
#define DICE_SPARQL_EXPRESSION_HPP

#include <Dice/rdf_tensor/Query.hpp>
#include <rdf4cpp/rdf.hpp>

namespace Dice::sparql2tensor::expressions {

	class Expression {
	public:
		Expression() = default;
		virtual ~Expression() = default;
		virtual void evaluate(rdf_tensor::Entry const &entry) = 0;
		[[nodiscard]] virtual std::optional<rdf4cpp::rdf::Node> result() const = 0;
		[[nodiscard]] virtual std::unique_ptr<Expression> clone() const = 0;
		[[nodiscard]] virtual std::vector<rdf4cpp::rdf::query::Variable> variables() const = 0;
	};

	class ExpressionList {
	private:
		std::vector<std::unique_ptr<Expression>> expressions_;
	public:
		ExpressionList() = default;
		explicit ExpressionList(std::vector<std::unique_ptr<Expression>> expressions);
		[[nodiscard]] std::vector<std::unique_ptr<Expression>> const &expressions() const;
		std::vector<std::unique_ptr<Expression>> &expressions();
		[[nodiscard]] ExpressionList clone() const;
	};

}//namespace Dice::sparql2tensor::expressions

#endif//DICE_SPARQL_EXPRESSION_HPP
