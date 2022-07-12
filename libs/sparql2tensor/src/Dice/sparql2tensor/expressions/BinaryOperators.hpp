#ifndef DICE_SPARQL_BINARYOPERERATORS_HPP
#define DICE_SPARQL_BINARYOPERERATORS_HPP

#include "Expression.hpp"

namespace Dice::sparql2tensor::expressions {

	/* General n-ary case of the ConditionalOr expression (https://www.w3.org/TR/sparql11-query/#rConditionalOrExpression) */
	class LogicalOrExpression : public Expression {
	private:
		std::vector<std::unique_ptr<Expression>> op_expressions_;
	public:
		explicit LogicalOrExpression(std::vector<std::unique_ptr<Expression>> op_expressions);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	/* General n-ary case of the ConditionalAnd expression (https://www.w3.org/TR/sparql11-query/#rConditionalAndExpression) */
	class LogicalAndExpression : public Expression {
	private:
		std::vector<std::unique_ptr<Expression>> op_expressions_;
	public:
		explicit LogicalAndExpression(std::vector<std::unique_ptr<Expression>> op_expressions);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	/* Equals RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class EqualsExpression : public Expression {
	private:
		std::unique_ptr<Expression> lhs_op_;
		std::unique_ptr<Expression> rhs_op_;
	public:
		explicit EqualsExpression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	/* NotEquals RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class NotEqualsExpression : public Expression {
	private:
		std::unique_ptr<Expression> lhs_op_;
		std::unique_ptr<Expression> rhs_op_;
	public:
		explicit NotEqualsExpression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	/* Less RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class LessExpression : public Expression {
	private:
		std::unique_ptr<Expression> lhs_op_;
		std::unique_ptr<Expression> rhs_op_;
	public:
		explicit LessExpression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	/* Greater RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class GreaterExpression : public Expression {
	private:
		std::unique_ptr<Expression> lhs_op_;
		std::unique_ptr<Expression> rhs_op_;
	public:
		explicit GreaterExpression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	/* LessEquals RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class LessEqualsExpression : public Expression {
	private:
		std::unique_ptr<Expression> lhs_op_;
		std::unique_ptr<Expression> rhs_op_;
	public:
		explicit LessEqualsExpression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	/* GreaterEquals RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class GreaterEqualsExpression : public Expression {
	private:
		std::unique_ptr<Expression> lhs_op_;
		std::unique_ptr<Expression> rhs_op_;
	public:
		explicit GreaterEqualsExpression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	/* In RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class InExpression : public Expression {
	private:
		std::unique_ptr<Expression> lhs_op_;
		ExpressionList rhs_op_;
	public:
		explicit InExpression(std::unique_ptr<Expression> lhs, ExpressionList rhs);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	/* NotIn RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class NotInExpression : public Expression {
	private:
		std::unique_ptr<Expression> lhs_op_;
		ExpressionList rhs_op_;
	public:
		explicit NotInExpression(std::unique_ptr<Expression> lhs, ExpressionList rhs);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

}//namespace Dice::sparql2tensor::expressions


#endif//DICE_SPARQL_BINARYOPERERATORS_HPP
