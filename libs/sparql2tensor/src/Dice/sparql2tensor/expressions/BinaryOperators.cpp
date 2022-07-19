#include "BinaryOperators.hpp"

namespace Dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;

	/* LogicalOr Operator */
	LogicalOrExpression::LogicalOrExpression(std::vector<std::unique_ptr<SPARQLExpression>> op_expressions)
		: op_expressions_(std::move(op_expressions)) {}

	void LogicalOrExpression::update_value(rdf_tensor::Entry const &entry) {
		for (auto const &expr : op_expressions_) {
			expr->update_value(entry);
		}
	}

	rdf_tensor::NodeWrapper LogicalOrExpression::evaluate() const {
//		auto result = std::any_of(op_expressions_.begin(), op_expressions_.end(),
//								  [](std::unique_ptr<Expression> const &Expr) {
//									  return true;
//								  });
		auto result = true;
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<SPARQLExpression> LogicalOrExpression::clone_sparql() const {
		std::vector<std::unique_ptr<SPARQLExpression>> clones{};
		for (auto const &expr : op_expressions_) {
			clones.push_back(expr->clone_sparql());
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
	LogicalAndExpression::LogicalAndExpression(std::vector<std::unique_ptr<SPARQLExpression>> expressions)
		: op_expressions_(std::move(expressions)) {}

	void LogicalAndExpression::update_value(rdf_tensor::Entry const &entry) {
		for (auto const &expr : op_expressions_) {
			expr->update_value(entry);
		}
	}

	rdf_tensor::NodeWrapper LogicalAndExpression::evaluate() const {
//		auto result = std::all_of(op_expressions_.begin(), op_expressions_.end(),
//								  [](std::unique_ptr<Expression> const &Expr) {
//									  return true;
//								  });
        auto result = true;
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<SPARQLExpression> LogicalAndExpression::clone_sparql() const {
		std::vector<std::unique_ptr<SPARQLExpression>> clones{};
		for (auto const &expr : op_expressions_) {
			clones.push_back(expr->clone_sparql());
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
	EqualsExpression::EqualsExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void EqualsExpression::update_value(rdf_tensor::Entry const &entry) {
		lhs_op_->update_value(entry);
		rhs_op_->update_value(entry);
	}

	rdf_tensor::NodeWrapper EqualsExpression::evaluate() const {
		auto lhs_res = lhs_op_->evaluate();
		auto rhs_res = rhs_op_->evaluate();
		auto result = (lhs_res == rhs_res);
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<SPARQLExpression> EqualsExpression::clone_sparql() const {
		return std::make_unique<EqualsExpression>(lhs_op_->clone_sparql(), rhs_op_->clone_sparql());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> EqualsExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* NotEquals Operator */
	NotEqualsExpression::NotEqualsExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void NotEqualsExpression::update_value(rdf_tensor::Entry const &entry) {
		lhs_op_->update_value(entry);
		rhs_op_->update_value(entry);
	}

	rdf_tensor::NodeWrapper NotEqualsExpression::evaluate() const {
		auto lhs_res = lhs_op_->evaluate();
		auto rhs_res = rhs_op_->evaluate();
		auto result = (lhs_res != rhs_res);
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<SPARQLExpression> NotEqualsExpression::clone_sparql() const {
		return std::make_unique<NotEqualsExpression>(lhs_op_->clone_sparql(), rhs_op_->clone_sparql());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> NotEqualsExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* Less Operator */
	LessExpression::LessExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void LessExpression::update_value(rdf_tensor::Entry const &entry) {
		lhs_op_->update_value(entry);
		rhs_op_->update_value(entry);
	}

	rdf_tensor::NodeWrapper LessExpression::evaluate() const {
		auto lhs_res = lhs_op_->evaluate();
		auto rhs_res = rhs_op_->evaluate();
		auto result = (lhs_res < rhs_res);
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<SPARQLExpression> LessExpression::clone_sparql() const {
		return std::make_unique<LessExpression>(lhs_op_->clone_sparql(), rhs_op_->clone_sparql());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LessExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* Greater Operator */
	GreaterExpression::GreaterExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void GreaterExpression::update_value(rdf_tensor::Entry const &entry) {
		lhs_op_->update_value(entry);
		rhs_op_->update_value(entry);
	}

	rdf_tensor::NodeWrapper GreaterExpression::evaluate() const {
		auto lhs_res = lhs_op_->evaluate();
		auto rhs_res = rhs_op_->evaluate();
		auto result = (lhs_res > rhs_res);
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<SPARQLExpression> GreaterExpression::clone_sparql() const {
		return std::make_unique<GreaterExpression>(lhs_op_->clone_sparql(), rhs_op_->clone_sparql());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> GreaterExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* LessEquals Operator */
	LessEqualsExpression::LessEqualsExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void LessEqualsExpression::update_value(rdf_tensor::Entry const &entry) {
		lhs_op_->update_value(entry);
		rhs_op_->update_value(entry);
	}

	rdf_tensor::NodeWrapper LessEqualsExpression::evaluate() const {
		auto lhs_res = lhs_op_->evaluate();
		auto rhs_res = rhs_op_->evaluate();
		auto result = (lhs_res <= rhs_res);
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<SPARQLExpression> LessEqualsExpression::clone_sparql() const {
		return std::make_unique<LessEqualsExpression>(lhs_op_->clone_sparql(), rhs_op_->clone_sparql());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LessEqualsExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* GreaterEquals Operator */
	GreaterEqualsExpression::GreaterEqualsExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void GreaterEqualsExpression::update_value(rdf_tensor::Entry const &entry) {
		lhs_op_->update_value(entry);
		rhs_op_->update_value(entry);
	}

	rdf_tensor::NodeWrapper GreaterEqualsExpression::evaluate() const {
		auto lhs_res = lhs_op_->evaluate();
		auto rhs_res = rhs_op_->evaluate();
		auto result = (lhs_res >= rhs_res);
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::unique_ptr<SPARQLExpression> GreaterEqualsExpression::clone_sparql() const {
		return std::make_unique<GreaterEqualsExpression>(lhs_op_->clone_sparql(), rhs_op_->clone_sparql());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> GreaterEqualsExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

//	/* In Operator */
//	InExpression::InExpression(std::unique_ptr<Expression> lhs, ExpressionList rhs)
//		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}
//
//	void InExpression::update_value(rdf_tensor::Entry const &entry) {
//		lhs_op_->evaluate(entry);
//		for (auto &expr : rhs_op_.expressions()) {
//			expr->update_value(entry);
//		}
//	}
//
//	rdf_tensor::NodeWrapper InExpression::evaluate() const {
//		bool contains_error = false;
//		auto lhs_result = lhs_op_->evaluate();
//		for (auto const &expr : rhs_op_.expressions()) {
//			auto expr_res = expr->evaluate();
//			if (not expr_res.has_value()) {
//				contains_error = true;
//				continue;
//			}
//			if (lhs_result == expr_res.value())
//				return Literal{"true", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
//		}
//		if (contains_error)
//			return std::nullopt;
//		return Literal{"false", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
//	}
//
//	std::unique_ptr<Expression> InExpression::clone_sparql() const {
//		return std::make_unique<InExpression>(lhs_op_->clone_sparql(), rhs_op_.clone_sparql());
//	}
//
//	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> InExpression::variables() const {
//		return lhs_op_->variables();
//	}
//
//	/* NotIn Operator */
//	NotInExpression::NotInExpression(std::unique_ptr<Expression> lhs, ExpressionList rhs)
//		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}
//
//	void NotInExpression::update_value(rdf_tensor::Entry const &entry) {
//		lhs_op_->evaluate(entry);
//		for (auto &expr : rhs_op_.expressions()) {
//			expr->update_value(entry);
//		}
//	}
//
//	rdf_tensor::NodeWrapper NotInExpression::evaluate() const {
//		bool contains_error = false;
//		auto lhs_result = lhs_op_->evaluate();
//		for (auto const &expr : rhs_op_.expressions()) {
//			auto expr_res = expr->evaluate();
//			if (not expr_res.has_value()) {
//				contains_error = true;
//				continue;
//			}
//			if (lhs_result == expr_res.value())
//				return Literal{"false", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
//		}
//		if (contains_error)
//			return std::nullopt;
//		return Literal{"true", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
//	}
//
//	std::unique_ptr<Expression> NotInExpression::clone_sparql() const {
//		return std::make_unique<NotInExpression>(lhs_op_->clone_sparql(), rhs_op_.clone_sparql());
//	}
//
//	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> NotInExpression::variables() const {
//		return lhs_op_->variables();
//	}

}// namespace Dice::sparql2tensor::expressions