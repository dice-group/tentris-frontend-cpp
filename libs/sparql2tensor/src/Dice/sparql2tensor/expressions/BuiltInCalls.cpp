#include "BuiltInCalls.hpp"

namespace Dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	/* IsIRI Expression */
	IsIRI::IsIRI(std::unique_ptr<Expression> op_expr)
		: op_expr_(std::move(op_expr)) {}


	void IsIRI::evaluate(const rdf_tensor::Entry &entry) {
		op_expr_->evaluate(entry);
	}

	std::optional<Node> IsIRI::result() const {
		auto expr_res = op_expr_->result();
		if (not expr_res.has_value())
			return std::nullopt;
		return Literal(std::to_string(expr_res.value().is_iri()),
									 IRI(IRI("http://www.w3.org/2001/XMLSchema#boolean")));
	}

	std::unique_ptr<Expression> IsIRI::clone() const {
		return std::make_unique<IsIRI>(op_expr_->clone());
	}

	std::vector<Variable> IsIRI::variables() const {
		return op_expr_->variables();
	}

	/* IsBlank Expression */
	IsBlank::IsBlank(std::unique_ptr<Expression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	void IsBlank::evaluate(const rdf_tensor::Entry &entry) {
		op_expr_->evaluate(entry);
	}

	std::optional<Node> IsBlank::result() const {
		auto expr_res = op_expr_->result();
		if (not expr_res.has_value())
			return std::nullopt;
		return Literal(std::to_string(expr_res.value().is_blank_node()),
									 IRI(IRI("http://www.w3.org/2001/XMLSchema#boolean")));
	}

	std::unique_ptr<Expression> IsBlank::clone() const {
		return std::make_unique<IsBlank>(op_expr_->clone());
	}

	std::vector<Variable> IsBlank::variables() const {
		return op_expr_->variables();
	}

	/* IsLiteral Expression */
	IsLiteral::IsLiteral(std::unique_ptr<Expression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	void IsLiteral::evaluate(const rdf_tensor::Entry &entry) {
		op_expr_->evaluate(entry);
	}

	std::optional<Node> IsLiteral::result() const {
		auto expr_res = op_expr_->result();
		if (not expr_res.has_value())
			return std::nullopt;
		return Literal(std::to_string(expr_res.value().is_literal()),
									 IRI(IRI("http://www.w3.org/2001/XMLSchema#boolean")));
	}

	std::unique_ptr<Expression> IsLiteral::clone() const {
		return std::make_unique<IsLiteral>(op_expr_->clone());
	}

	std::vector<Variable> IsLiteral::variables() const {
		return op_expr_->variables();
	}

	/* Datatype Expression */
	Datatype::Datatype(std::unique_ptr<Expression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	void Datatype::evaluate(const rdf_tensor::Entry &entry) {
		op_expr_->evaluate(entry);
	}

	std::optional<Node> Datatype::result() const {
		auto expr_res = op_expr_->result();
		if (not expr_res.has_value())
			return std::nullopt;
		if (not expr_res.value().is_literal())
			return std::nullopt;
		return static_cast<Literal>(expr_res.value()).datatype();
	}

	std::unique_ptr<Expression> Datatype::clone() const {
		return std::make_unique<Datatype>(op_expr_->clone());
	}

	std::vector<Variable> Datatype::variables() const {
		return op_expr_->variables();
	}

}// namespace Dice::sparql2tensor::expressions