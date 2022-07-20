#ifndef DICE_SPARQL_BINARYOPERERATORS_HPP
#define DICE_SPARQL_BINARYOPERERATORS_HPP

#include "Expression.hpp"

namespace Dice::sparql2tensor::expressions {

	/* General n-ary case of the ConditionalAnd expression (https://www.w3.org/TR/sparql11-query/#rConditionalAndExpression) */
	class LogicalAndExpression : public SPARQLExpression {
	private:
		std::vector<std::unique_ptr<SPARQLExpression>> op_expressions_;
	public:
		explicit LogicalAndExpression(std::vector<std::unique_ptr<SPARQLExpression>> op_expressions);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<std::unique_ptr<SPARQLExpression>> &expressions();
	protected:
		[[nodiscard]] LogicalAndExpression *clone_impl() const override;
	};

	/* General n-ary case of the ConditionalOr expression (https://www.w3.org/TR/sparql11-query/#rConditionalOrExpression) */
	class LogicalOrExpression : public SPARQLExpression {
	private:
		std::vector<std::unique_ptr<SPARQLExpression>> op_expressions_;
	public:
		explicit LogicalOrExpression(std::vector<std::unique_ptr<SPARQLExpression>> op_expressions);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] LogicalOrExpression *clone_impl() const override;
	};

	/* Equals RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class EqualsExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> lhs_op_;
		std::unique_ptr<SPARQLExpression> rhs_op_;
	public:
		explicit EqualsExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] EqualsExpression *clone_impl() const override;
	};

	/* NotEquals RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class NotEqualsExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> lhs_op_;
		std::unique_ptr<SPARQLExpression> rhs_op_;
	public:
		explicit NotEqualsExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] NotEqualsExpression *clone_impl() const override;
	};

	/* Less RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class LessExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> lhs_op_;
		std::unique_ptr<SPARQLExpression> rhs_op_;
	public:
		explicit LessExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] LessExpression *clone_impl() const override;
	};

	/* Greater RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class GreaterExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> lhs_op_;
		std::unique_ptr<SPARQLExpression> rhs_op_;
	public:
		explicit GreaterExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] GreaterExpression *clone_impl() const override;
	};

	/* LessEquals RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class LessEqualsExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> lhs_op_;
		std::unique_ptr<SPARQLExpression> rhs_op_;
	public:
		explicit LessEqualsExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] LessEqualsExpression *clone_impl() const override;
	};

	/* GreaterEquals RelationalExpression (https://www.w3.org/TR/sparql11-query/#rRelationalExpression) */
	class GreaterEqualsExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> lhs_op_;
		std::unique_ptr<SPARQLExpression> rhs_op_;
	public:
		explicit GreaterEqualsExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] GreaterEqualsExpression *clone_impl() const override;
	};

}//namespace Dice::sparql2tensor::expressions


#endif//DICE_SPARQL_BINARYOPERERATORS_HPP
