#include "LogicalOperators.hpp"

namespace dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;

	/* LogicalAnd Operator */
	LogicalAndExpression::LogicalAndExpression(std::vector<std::unique_ptr<SPARQLExpression>> op_expressions)
		: op_expressions_(std::move(op_expressions)) {}

	void LogicalAndExpression::update_value(rdf_tensor::Entry const &entry) {
		for (auto const &expr : op_expressions_) {
			expr->update_value(entry);
		}
	}

	rdf_tensor::NodeWrapper LogicalAndExpression::evaluate() const {
		bool contains_error = false;
		for (auto const &expr : op_expressions_) {
			auto expr_result = expr->evaluate();
			if (expr_result.null()) {
				contains_error = true;
				continue;
			}
			if (not bool(expr_result)) {
				return FalseLiteral::instance();
			}
		}
		if (contains_error)
			return {};
		return TrueLiteral::instance();
	}

	LogicalAndExpression *LogicalAndExpression::clone_impl() const {
		std::vector<std::unique_ptr<SPARQLExpression>> clones{};
		for (auto const &expr : op_expressions_) {
			clones.push_back(expr->clone());
		}
		return new LogicalAndExpression(std::move(clones));
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LogicalAndExpression::variables() const {
		auto variables = op_expressions_[0]->variables();
		for (size_t i = 1; i < op_expressions_.size(); i++) {
			auto vars = op_expressions_[i]->variables();
			variables.insert(variables.end(), vars.begin(), vars.end());
		}
		return variables;
	}

	[[nodiscard]] std::vector<std::unique_ptr<SPARQLExpression>> &LogicalAndExpression::expressions() {
		return op_expressions_;
	}

	/* LogicalOr Operator */
	LogicalOrExpression::LogicalOrExpression(std::vector<std::unique_ptr<SPARQLExpression>> op_expressions)
		: op_expressions_(std::move(op_expressions)) {}

	void LogicalOrExpression::update_value(rdf_tensor::Entry const &entry) {
		for (auto const &expr : op_expressions_) {
			expr->update_value(entry);
		}
	}

	rdf_tensor::NodeWrapper LogicalOrExpression::evaluate() const {
		bool contains_error = false;
		for (auto const &expr : op_expressions_) {
			auto expr_result = expr->evaluate();
			if (expr_result.null()) {
				contains_error = true;
				continue;
			}
			if (not bool(expr_result)) {
				return TrueLiteral::instance();
			}
		}
		if (contains_error)
			return {};
		return FalseLiteral::instance();
	}

	LogicalOrExpression *LogicalOrExpression::clone_impl() const {
		std::vector<std::unique_ptr<SPARQLExpression>> clones{};
		for (auto const &expr : op_expressions_) {
			clones.push_back(expr->clone());
		}
		return new LogicalOrExpression(std::move(clones));
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LogicalOrExpression::variables() const {
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
		if (result) return TrueLiteral::instance();
		return FalseLiteral::instance();
	}

	EqualsExpression *EqualsExpression::clone_impl() const {
		return new EqualsExpression(lhs_op_->clone(), rhs_op_->clone());
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
		if (result) return TrueLiteral::instance();
		return FalseLiteral::instance();
	}

	NotEqualsExpression *NotEqualsExpression::clone_impl() const {
		return new NotEqualsExpression(lhs_op_->clone(), rhs_op_->clone());
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
		if (result) return TrueLiteral::instance();
		return FalseLiteral::instance();
	}

	LessExpression *LessExpression::clone_impl() const {
		return new LessExpression(lhs_op_->clone(), rhs_op_->clone());
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
		if (result) return TrueLiteral::instance();
		return FalseLiteral::instance();
	}

	GreaterExpression *GreaterExpression::clone_impl() const {
		return new GreaterExpression(lhs_op_->clone(), rhs_op_->clone());
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
		if (result) return TrueLiteral::instance();
		return FalseLiteral::instance();
	}

	LessEqualsExpression *LessEqualsExpression::clone_impl() const {
		return new LessEqualsExpression(lhs_op_->clone(), rhs_op_->clone());
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
		if (result) return TrueLiteral::instance();
		return FalseLiteral::instance();
	}

	GreaterEqualsExpression *GreaterEqualsExpression::clone_impl() const {
		return new GreaterEqualsExpression(lhs_op_->clone(), rhs_op_->clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> GreaterEqualsExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

}// namespace dice::sparql2tensor::expressions