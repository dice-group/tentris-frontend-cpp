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
		auto expr_result = rdf_tensor::NodeWrapper(primary_expr_->evaluate());
		if (expr_result.null())
			return {};
		auto bool_result = not bool(expr_result); // boolean coercion
		return Literal{std::to_string(bool_result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	NotExpression *NotExpression::clone_impl() const {
		return new NotExpression(primary_expr_->clone());
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

	UnaryPlusExpression *UnaryPlusExpression::clone_impl() const {
		return new UnaryPlusExpression(primary_expr_->clone());
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

	UnaryMinusExpression *UnaryMinusExpression::clone_impl() const {
		return new UnaryMinusExpression(primary_expr_->clone());
	}

	[[nodiscard]] std::vector<Variable> UnaryMinusExpression::variables() const {
		return primary_expr_->variables();
	}


}// namespace Dice::sparql2tensor::expressions