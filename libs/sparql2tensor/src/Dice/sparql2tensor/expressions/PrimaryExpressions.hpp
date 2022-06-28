#ifndef DICE_SPARQL_PRIMARYEXPRESSIONS_HPP
#define DICE_SPARQL_PRIMARYEXPRESSIONS_HPP

#include "Expression.hpp"

namespace Dice::sparql2tensor::expressions {

	/* PrimaryExpression for Variables (https://www.w3.org/TR/sparql11-query/#rPrimaryExpression) */
	class PrimaryVarExpression : public Expression {
	private:
		size_t var_pos_in_entry_;
		rdf4cpp::rdf::Node rdf_node_;
		rdf4cpp::rdf::query::Variable variable_;
	public:
		explicit PrimaryVarExpression(rdf4cpp::rdf::query::Variable variable, size_t var_pos_in_entry);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	/* PrimaryExpression for RDFLiterals, NumericLiterals and BooleanLiterals (https://www.w3.org/TR/sparql11-query/#rPrimaryExpression) */
	struct PrimaryLiteralExpression : public Expression {
	private:
		rdf4cpp::rdf::Literal literal_;
	public:
		explicit PrimaryLiteralExpression(rdf4cpp::rdf::Literal literal);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	/* PrimaryExpression for BuiltInCalls (https://www.w3.org/TR/sparql11-query/#rPrimaryExpression) */
	struct PrimaryBuiltInCallExpression : public Expression {
	private:
		std::unique_ptr<Expression> built_in_call_;
	public:
		explicit PrimaryBuiltInCallExpression(std::unique_ptr<Expression> built_in_call);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

}//namespace Dice::sparql2tensor::expressions


#endif//DICE_SPARQL_PRIMARYEXPRESSIONS_HPP
