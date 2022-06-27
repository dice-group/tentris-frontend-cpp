#include "BinaryOperators.hpp"

namespace Dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;

	/* LogicalOr Operator */
	LogicalOrExpression::LogicalOrExpression(std::vector<std::unique_ptr<Expression>> op_expressions)
		: op_expressions_(std::move(op_expressions)) {}

	void LogicalOrExpression::evaluate(rdf_tensor::Entry const &entry) {
		for (auto const &expr : op_expressions_) {
			expr->evaluate(entry);
		}
	}

	Node LogicalOrExpression::result() const {
		auto result = std::any_of(op_expressions_.begin(), op_expressions_.end(),
								  [](std::unique_ptr<Expression> const &Expr) {
									  return true;
								  });
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<Expression> LogicalOrExpression::clone() const {
		std::vector<std::unique_ptr<Expression>> clones{};
		for (auto const &expr : op_expressions_) {
			clones.push_back(expr->clone());
		}
		return std::make_unique<LogicalOrExpression>(std::move(clones));
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LogicalOrExpression::variables() const {
		auto variables = op_expressions_[0]->variables();
		for (size_t i = 1; i < op_expressions_.size(); i++) {
			auto vars = op_expressions_[i]->variables();
			variables.insert(variables.end(), vars.begin(), vars.end());
		}
		return variables;
	}

	/* LogicalAnd Operator */
	LogicalAndExpression::LogicalAndExpression(std::vector<std::unique_ptr<Expression>> expressions)
		: op_expressions_(std::move(expressions)) {}

	void LogicalAndExpression::evaluate(rdf_tensor::Entry const &entry) {
		for (auto const &expr : op_expressions_) {
			expr->evaluate(entry);
		}
	}

	Node LogicalAndExpression::result() const {
		auto result = std::all_of(op_expressions_.begin(), op_expressions_.end(),
								  [](std::unique_ptr<Expression> const &Expr) {
									  return true;
								  });
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<Expression> LogicalAndExpression::clone() const {
		std::vector<std::unique_ptr<Expression>> clones{};
		for (auto const &expr : op_expressions_) {
			clones.push_back(expr->clone());
		}
		return std::make_unique<LogicalAndExpression>(std::move(clones));
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LogicalAndExpression::variables() const {
		auto variables = op_expressions_[0]->variables();
		for (size_t i = 1; i < op_expressions_.size(); i++) {
			auto vars = op_expressions_[i]->variables();
			variables.insert(variables.end(), vars.begin(), vars.end());
		}
		return variables;
	}

	/* Equals Operator */
	EqualsExpression::EqualsExpression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void EqualsExpression::evaluate(rdf_tensor::Entry const &entry) {
		lhs_op_->evaluate(entry);
		rhs_op_->evaluate(entry);
	}

	Node EqualsExpression::result() const {
		auto result = (lhs_op_->result() == rhs_op_->result());
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<Expression> EqualsExpression::clone() const {
		return std::make_unique<EqualsExpression>(lhs_op_->clone(), rhs_op_->clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> EqualsExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* NotEquals Operator */
	NotEqualsExpression::NotEqualsExpression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void NotEqualsExpression::evaluate(rdf_tensor::Entry const &entry) {
		lhs_op_->evaluate(entry);
		rhs_op_->evaluate(entry);
	}

	Node NotEqualsExpression::result() const {
		auto result = (lhs_op_->result() != rhs_op_->result());
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<Expression> NotEqualsExpression::clone() const {
		return std::make_unique<NotEqualsExpression>(lhs_op_->clone(), rhs_op_->clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> NotEqualsExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* Less Operator */
	LessExpression::LessExpression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void LessExpression::evaluate(rdf_tensor::Entry const &entry) {
		lhs_op_->evaluate(entry);
		rhs_op_->evaluate(entry);
	}

	Node LessExpression::result() const {
		auto result = (lhs_op_->result() < rhs_op_->result());
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<Expression> LessExpression::clone() const {
		return std::make_unique<LessExpression>(lhs_op_->clone(), rhs_op_->clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LessExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* Greater Operator */
	GreaterExpression::GreaterExpression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void GreaterExpression::evaluate(rdf_tensor::Entry const &entry) {
		lhs_op_->evaluate(entry);
		rhs_op_->evaluate(entry);
	}

	Node GreaterExpression::result() const {
		auto result = (lhs_op_->result() > rhs_op_->result());
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<Expression> GreaterExpression::clone() const {
		return std::make_unique<GreaterExpression>(lhs_op_->clone(), rhs_op_->clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> GreaterExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* LessEquals Operator */
	LessEqualsExpression::LessEqualsExpression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void LessEqualsExpression::evaluate(rdf_tensor::Entry const &entry) {
		lhs_op_->evaluate(entry);
		rhs_op_->evaluate(entry);
	}

	Node LessEqualsExpression::result() const {
		auto result = (lhs_op_->result() <= rhs_op_->result());
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<Expression> LessEqualsExpression::clone() const {
		return std::make_unique<LessEqualsExpression>(lhs_op_->clone(), rhs_op_->clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LessEqualsExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* GreaterEquals Operator */
	GreaterEqualsExpression::GreaterEqualsExpression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void GreaterEqualsExpression::evaluate(rdf_tensor::Entry const &entry) {
		lhs_op_->evaluate(entry);
		rhs_op_->evaluate(entry);
	}

	Node GreaterEqualsExpression::result() const {
		auto result = (lhs_op_->result() >= rhs_op_->result());
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<Expression> GreaterEqualsExpression::clone() const {
		return std::make_unique<GreaterEqualsExpression>(lhs_op_->clone(), rhs_op_->clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> GreaterEqualsExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* In Operator */
	InExpression::InExpression(std::unique_ptr<Expression> lhs, ExpressionList rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void InExpression::evaluate(rdf_tensor::Entry const &entry) {
		lhs_op_->evaluate(entry);
		for (auto &expr : rhs_op_.expressions()) {
			expr->evaluate(entry);
		}
	}

	Node InExpression::result() const {
		auto lhs_result = lhs_op_->result();
		for (auto const &expr : rhs_op_.expressions()) {
			if (lhs_result == expr->result())
				return Literal{"true", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
		}
		return Literal{"false", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<Expression> InExpression::clone() const {
		return std::make_unique<InExpression>(lhs_op_->clone(), rhs_op_.clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> InExpression::variables() const {
		return lhs_op_->variables();
	}

	/* NotIn Operator */
	NotInExpression::NotInExpression(std::unique_ptr<Expression> lhs, ExpressionList rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void NotInExpression::evaluate(rdf_tensor::Entry const &entry) {
		lhs_op_->evaluate(entry);
		for (auto &expr : rhs_op_.expressions()) {
			expr->evaluate(entry);
		}
	}

	Node NotInExpression::result() const {
		auto lhs_result = lhs_op_->result();
		for (auto const &expr : rhs_op_.expressions()) {
			if (lhs_result == expr->result())
				return Literal{"false", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
		}
		return Literal{"true", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<Expression> NotInExpression::clone() const {
		return std::make_unique<NotInExpression>(lhs_op_->clone(), rhs_op_.clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> NotInExpression::variables() const {
		return lhs_op_->variables();
	}

}// namespace Dice::sparql2tensor::expressions