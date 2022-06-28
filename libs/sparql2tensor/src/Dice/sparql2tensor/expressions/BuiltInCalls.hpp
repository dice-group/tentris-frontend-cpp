#ifndef DICE_SPARQL_BUILTINCALLS_HPP
#define DICE_SPARQL_BUILTINCALLS_HPP

#include "Expression.hpp"

namespace Dice::sparql2tensor::expressions {

	class IsIRI : public Expression {
	private:
		std::unique_ptr<Expression> op_expr_;
	public:
		explicit IsIRI(std::unique_ptr<Expression> op_expr);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	class IsBlank : public Expression {
	private:
		std::unique_ptr<Expression> op_expr_;
	public:
		explicit IsBlank(std::unique_ptr<Expression> op_expr);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	class IsLiteral : public Expression {
	private:
		std::unique_ptr<Expression> op_expr_;
	public:
		explicit IsLiteral(std::unique_ptr<Expression> op_expr);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	class Datatype : public Expression {
	private:
		std::unique_ptr<Expression> op_expr_;
	public:
		explicit Datatype(std::unique_ptr<Expression> op_expr);
		void evaluate(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] std::optional<rdf4cpp::rdf::Node> result() const override;
		[[nodiscard]] std::unique_ptr<Expression> clone() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

}//namespace Dice::sparql2tensor::expressions

#endif//DICE_SPARQL_BUILTINCALLS_HPP
