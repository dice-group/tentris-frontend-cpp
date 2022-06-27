#include "BuiltInCalls.hpp"

namespace Dice::sparql2tensor::expressions {

	/* IsIRI Expression */
	IsIRI::IsIRI(std::unique_ptr<Expression> op_expr)
		: op_expr_(std::move(op_expr)) {}


	void IsIRI::evaluate(const rdf_tensor::Entry &entry) {
		op_expr_->evaluate(entry);
	}

	rdf4cpp::rdf::Node IsIRI::result() const {
		auto expr_res = op_expr_->result();
		return rdf4cpp::rdf::Literal(std::to_string(expr_res.is_iri()),
									 rdf4cpp::rdf::IRI(rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")));
	}

	std::unique_ptr<Expression> IsIRI::clone() const {
		return std::make_unique<IsIRI>(op_expr_->clone());
	}

	std::vector<rdf4cpp::rdf::query::Variable> IsIRI::variables() const {
		return op_expr_->variables();
	}

}// namespace Dice::sparql2tensor::expressions