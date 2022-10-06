#ifndef DICE_SPARQL_EXPRESSION_HPP
#define DICE_SPARQL_EXPRESSION_HPP

#include <dice/rdf-tensor/Query.hpp>
#include <rdf4cpp/rdf.hpp>

namespace dice::sparql2tensor::expressions {

	class SPARQLExpression : public rdf_tensor::Expression {
	public:
		SPARQLExpression() = default;
		[[nodiscard]] std::unique_ptr<SPARQLExpression> clone() const { return std::unique_ptr<SPARQLExpression>(clone_impl()); }
		[[nodiscard]] virtual std::vector<rdf4cpp::rdf::query::Variable> variables() const = 0;

	protected:
		[[nodiscard]] virtual SPARQLExpression *clone_impl() const = 0;
	};

	class TrueLiteral {
	private:
		TrueLiteral() = default;

	public:
		static rdf4cpp::rdf::Literal instance() {
			static rdf4cpp::rdf::Literal true_literal{"true", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
			return true_literal;
		}
	};

	class FalseLiteral {
	private:
		FalseLiteral() = default;

	public:
		static rdf4cpp::rdf::Literal instance() {
			static rdf4cpp::rdf::Literal false_literal{"false", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
			return false_literal;
		}
	};

}//namespace dice::sparql2tensor::expressions

#endif//DICE_SPARQL_EXPRESSION_HPP
