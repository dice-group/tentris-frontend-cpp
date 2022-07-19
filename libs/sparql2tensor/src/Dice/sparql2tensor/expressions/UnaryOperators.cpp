#include "UnaryOperators.hpp"

namespace Dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	/* NotExpression Operator */
	NotExpression::NotExpression(std::unique_ptr<SPARQLExpression> primary_expr)
		: primary_expr_(std::move(primary_expr)) {}

	void NotExpression::update_value(rdf_tensor::Entry const &entry) {
		primary_expr_->update_value(entry);
	}

	rdf_tensor::NodeWrapper NotExpression::evaluate() const {
		return primary_expr_->evaluate(); // todo: coerce to boolean and apply not
	}

	std::unique_ptr<SPARQLExpression> NotExpression::clone_sparql() const {
		return std::make_unique<NotExpression>(primary_expr_->clone_sparql());
	}

	[[nodiscard]] std::vector<Variable> NotExpression::variables() const {
		return primary_expr_->variables();
	}


	/* UnaryPlusExpression Operator */
	UnaryPlusExpression::UnaryPlusExpression(std::unique_ptr<SPARQLExpression> primary_expr)
		: primary_expr_(std::move(primary_expr)) {}

	void UnaryPlusExpression::update_value(rdf_tensor::Entry const &entry) {
		return primary_expr_->update_value(entry);
	}

	rdf_tensor::NodeWrapper UnaryPlusExpression::evaluate() const {
		return primary_expr_->evaluate();
	}

	std::unique_ptr<SPARQLExpression> UnaryPlusExpression::clone_sparql() const {
		return std::make_unique<UnaryPlusExpression>(primary_expr_->clone_sparql());
	}

	[[nodiscard]] std::vector<Variable> UnaryPlusExpression::variables() const {
		return primary_expr_->variables();
	}

	/* UnaryMinusExpression Operator */
	UnaryMinusExpression::UnaryMinusExpression(std::unique_ptr<SPARQLExpression> primary_expr)
		: primary_expr_(std::move(primary_expr)) {}

	void UnaryMinusExpression::update_value(rdf_tensor::Entry const &entry) {
		primary_expr_->update_value(entry);
	}

	rdf_tensor::NodeWrapper UnaryMinusExpression::evaluate() const {
		return primary_expr_->evaluate(); // todo: get numerical and multiply by -1
	}

	std::unique_ptr<SPARQLExpression> UnaryMinusExpression::clone_sparql() const {
		return std::make_unique<UnaryMinusExpression>(primary_expr_->clone_sparql());
	}

	[[nodiscard]] std::vector<Variable> UnaryMinusExpression::variables() const {
		return primary_expr_->variables();
	}


}// namespace Dice::sparql2tensor::expressions