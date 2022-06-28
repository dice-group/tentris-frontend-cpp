#ifndef DICE_SPARQL_UNARYOPERATORS_HPP
#define DICE_SPARQL_UNARYOPERATORS_HPP

#include "Expression.hpp"

namespace Dice::sparql2tensor::expressions {

	/* https://www.w3.org/TR/xpath-functions/#func-not */
	class NotExpression : public Expression {
	private:
		std::unique_ptr<Expression> primary_expr_;
	public:
		explicit NotExpression(std::unique_ptr<Expression> primary_expr);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	/* https://www.w3.org/TR/xpath-functions/#func-numeric-unary-plus */
	class UnaryPlusExpression : public Expression {
	private:
		std::unique_ptr<Expression> primary_expr_;
	public:
		explicit UnaryPlusExpression(std::unique_ptr<Expression> primary_expr);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	/* https://www.w3.org/TR/xpath-functions/#func-numeric-unary-minus */
	class UnaryMinusExpression : public Expression {
	private:
		std::unique_ptr<Expression> primary_expr_;
	public:
		explicit UnaryMinusExpression(std::unique_ptr<Expression> primary_expr);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

} //namespace Dice::sparql2tensor::expressions


#endif//DICE_SPARQL_UNARYOPERATORS_HPP
