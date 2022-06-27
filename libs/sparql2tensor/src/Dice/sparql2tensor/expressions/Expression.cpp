#include "Expression.hpp"

namespace Dice::sparql2tensor::expressions {

	ExpressionList::ExpressionList(std::vector<std::unique_ptr<Expression>> expressions)
		: expressions_(std::move(expressions)) {}

	std::vector<std::unique_ptr<Expression>> const &ExpressionList::expressions() const {
		return expressions_;
	}
	std::vector<std::unique_ptr<Expression>> &ExpressionList::expressions() {
		return expressions_;
	}

	[[nodiscard]] ExpressionList ExpressionList::clone() const {
		std::vector<std::unique_ptr<Expression>> clones{};
		for (auto const &expr : expressions_) {
			clones.push_back(expr->clone());
		}
		return ExpressionList(std::move(clones));
	}

}// namespace Dice::sparql2tensor::expressions