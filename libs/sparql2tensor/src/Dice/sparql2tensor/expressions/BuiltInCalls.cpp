#include "BuiltInCalls.hpp"

namespace Dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	/* IsIRI Expression */
	IsIRI::IsIRI(std::unique_ptr<SPARQLExpression> op_expr)
		: op_expr_(std::move(op_expr)) {}


	void IsIRI::update_value(const rdf_tensor::Entry &entry) {
		op_expr_->update_value(entry);
	}

	rdf_tensor::NodeWrapper IsIRI::evaluate() const {
		auto expr_res = op_expr_->evaluate();
		return Literal(std::to_string(expr_res.is_iri()),
									 IRI(IRI("http://www.w3.org/2001/XMLSchema#boolean")));
	}

	std::unique_ptr<SPARQLExpression> IsIRI::clone_sparql() const {
		return std::make_unique<IsIRI>(op_expr_->clone_sparql());
	}

	std::vector<Variable> IsIRI::variables() const {
		return op_expr_->variables();
	}

	/* IsBlank Expression */
	IsBlank::IsBlank(std::unique_ptr<SPARQLExpression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	void IsBlank::update_value(const rdf_tensor::Entry &entry) {
		op_expr_->update_value(entry);
	}

	rdf_tensor::NodeWrapper IsBlank::evaluate() const {
		auto expr_res = op_expr_->evaluate();
		return Literal(std::to_string(expr_res.is_blank_node()),
									 IRI(IRI("http://www.w3.org/2001/XMLSchema#boolean")));
	}

	std::unique_ptr<SPARQLExpression> IsBlank::clone_sparql() const {
		return std::make_unique<IsBlank>(op_expr_->clone_sparql());
	}

	std::vector<Variable> IsBlank::variables() const {
		return op_expr_->variables();
	}

	/* IsLiteral Expression */
	IsLiteral::IsLiteral(std::unique_ptr<SPARQLExpression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	void IsLiteral::update_value(const rdf_tensor::Entry &entry) {
		op_expr_->update_value(entry);
	}

	rdf_tensor::NodeWrapper IsLiteral::evaluate() const {
		auto expr_res = op_expr_->evaluate();
		return Literal(std::to_string(expr_res.is_literal()),
									 IRI(IRI("http://www.w3.org/2001/XMLSchema#boolean")));
	}

	std::unique_ptr<SPARQLExpression> IsLiteral::clone_sparql() const {
		return std::make_unique<IsLiteral>(op_expr_->clone_sparql());
	}

	std::vector<Variable> IsLiteral::variables() const {
		return op_expr_->variables();
	}

	/* Datatype Expression */
	Datatype::Datatype(std::unique_ptr<SPARQLExpression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	void Datatype::update_value(const rdf_tensor::Entry &entry) {
		op_expr_->update_value(entry);
	}

	rdf_tensor::NodeWrapper Datatype::evaluate() const {
		auto expr_res = op_expr_->evaluate();
		if (not expr_res.is_literal())
			return rdf_tensor::NodeWrapper();
		return static_cast<Literal>(expr_res).datatype();
	}

	std::unique_ptr<SPARQLExpression> Datatype::clone_sparql() const {
		return std::make_unique<Datatype>(op_expr_->clone_sparql());
	}

	std::vector<Variable> Datatype::variables() const {
		return op_expr_->variables();
	}

}// namespace Dice::sparql2tensor::expressions