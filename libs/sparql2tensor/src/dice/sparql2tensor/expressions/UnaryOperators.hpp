#ifndef DICE_SPARQL_UNARYOPERATORS_HPP
#define DICE_SPARQL_UNARYOPERATORS_HPP

#include "Expression.hpp"

namespace dice::sparql2tensor::expressions {

	/* https://www.w3.org/TR/xpath-functions/#func-not */
	class NotExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> primary_expr_;
	public:
		explicit NotExpression(std::unique_ptr<SPARQLExpression> primary_expr);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] NotExpression *clone_impl() const override;
	};

	/* https://www.w3.org/TR/xpath-functions/#func-numeric-unary-plus */
	class UnaryPlusExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> primary_expr_;
	public:
		explicit UnaryPlusExpression(std::unique_ptr<SPARQLExpression> primary_expr);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] UnaryPlusExpression *clone_impl() const override;
	};

	/* https://www.w3.org/TR/xpath-functions/#func-numeric-unary-minus */
	class UnaryMinusExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> primary_expr_;
	public:
		explicit UnaryMinusExpression(std::unique_ptr<SPARQLExpression> primary_expr);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] UnaryMinusExpression *clone_impl() const override;
	};

} //namespace dice::sparql2tensor::expressions


#endif//DICE_SPARQL_UNARYOPERATORS_HPP
