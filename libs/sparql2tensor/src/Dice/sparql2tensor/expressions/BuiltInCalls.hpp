#ifndef DICE_SPARQL_BUILTINCALLS_HPP
#define DICE_SPARQL_BUILTINCALLS_HPP

#include "Expression.hpp"

namespace Dice::sparql2tensor::expressions {

	class IsIRI : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_;

	public:
		explicit IsIRI(std::unique_ptr<SPARQLExpression> op_expr);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] IsIRI *clone_impl() const override;
	};

	class IsBlank : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_;

	public:
		explicit IsBlank(std::unique_ptr<SPARQLExpression> op_expr);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] IsBlank *clone_impl() const override;
	};

	class IsLiteral : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_;

	public:
		explicit IsLiteral(std::unique_ptr<SPARQLExpression> op_expr);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] IsLiteral *clone_impl() const override;
	};

	class Datatype : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_;

	public:
		explicit Datatype(std::unique_ptr<SPARQLExpression> op_expr);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] Datatype *clone_impl() const override;
	};

}//namespace Dice::sparql2tensor::expressions

#endif//DICE_SPARQL_BUILTINCALLS_HPP
