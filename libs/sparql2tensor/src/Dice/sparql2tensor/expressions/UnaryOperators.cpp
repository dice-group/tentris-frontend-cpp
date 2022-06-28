#include "UnaryOperators.hpp"

namespace Dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	/* NotExpression Operator */
	NotExpression::NotExpression(std::unique_ptr<Expression> primary_expr)
		: primary_expr_(std::move(primary_expr)) {}

	void NotExpression::evaluate(rdf_tensor::Entry const &entry) {
		primary_expr_->evaluate(entry);
	}

	std::optional<Node> NotExpression::result() const {
		return primary_expr_->result(); // todo: coerce to boolean and apply not
	}

	std::unique_ptr<Expression> NotExpression::clone() const {
		return std::make_unique<NotExpression>(primary_expr_->clone());
	}

	[[nodiscard]] std::vector<Variable> NotExpression::variables() const {
		return primary_expr_->variables();
	}


	/* UnaryPlusExpression Operator */
	UnaryPlusExpression::UnaryPlusExpression(std::unique_ptr<Expression> primary_expr)
		: primary_expr_(std::move(primary_expr)) {}

	void UnaryPlusExpression::evaluate([[maybe_unused]] rdf_tensor::Entry const &entry) {
		return primary_expr_->evaluate(entry);
	}

	std::optional<Node> UnaryPlusExpression::result() const {
		return primary_expr_->result();
	}

	std::unique_ptr<Expression> UnaryPlusExpression::clone() const {
		return std::make_unique<UnaryPlusExpression>(primary_expr_->clone());
	}

	[[nodiscard]] std::vector<Variable> UnaryPlusExpression::variables() const {
		return primary_expr_->variables();
	}

	/* UnaryMinusExpression Operator */
	UnaryMinusExpression::UnaryMinusExpression(std::unique_ptr<Expression> primary_expr)
		: primary_expr_(std::move(primary_expr)) {}

	void UnaryMinusExpression::evaluate(rdf_tensor::Entry const &entry) {
		primary_expr_->evaluate(entry);
	}

	std::optional<Node> UnaryMinusExpression::result() const {
		return primary_expr_->result(); // todo: get numerical and multiply by -1
	}

	std::unique_ptr<Expression> UnaryMinusExpression::clone() const {
		return std::make_unique<UnaryMinusExpression>(primary_expr_->clone());
	}

	[[nodiscard]] std::vector<Variable> UnaryMinusExpression::variables() const {
		return primary_expr_->variables();
	}


}// namespace Dice::sparql2tensor::expressions