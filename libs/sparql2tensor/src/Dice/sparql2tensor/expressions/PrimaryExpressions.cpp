#include "PrimaryExpressions.hpp"

namespace Dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	/* Variable Expression */
	PrimaryVarExpression::PrimaryVarExpression(Variable variable, size_t var_pos_in_entry)
		: var_pos_in_entry_(var_pos_in_entry), rdf_node_(), variable_(variable) {}

	void PrimaryVarExpression::evaluate(rdf_tensor::Entry const &entry) {
		rdf_node_ = entry[var_pos_in_entry_];
	}

	std::optional<Node> PrimaryVarExpression::result() const {
		return rdf_node_;
	}

	std::unique_ptr<Expression> PrimaryVarExpression::clone() const {
		return std::make_unique<PrimaryVarExpression>(*this);
	}

	std::vector<Variable> PrimaryVarExpression::variables() const {
		return {variable_};
	}

	/* Literal Expression */
	PrimaryLiteralExpression::PrimaryLiteralExpression(Literal literal)
		: literal_(literal) {}

	void PrimaryLiteralExpression::evaluate([[maybe_unused]] rdf_tensor::Entry const &entry) {}

	std::optional<Node> PrimaryLiteralExpression::result() const {
		return literal_;
	}

	std::unique_ptr<Expression> PrimaryLiteralExpression::clone() const {
		return std::make_unique<PrimaryLiteralExpression>(*this);
	}

	std::vector<Variable> PrimaryLiteralExpression::variables() const {
		return {};
	}

	/* BuiltInCall Expression */
	PrimaryBuiltInCallExpression::PrimaryBuiltInCallExpression(std::unique_ptr<Expression> expr)
		: built_in_call_(std::move(expr)) {}

	void PrimaryBuiltInCallExpression::evaluate(rdf_tensor::Entry const &entry) {
		return built_in_call_->evaluate(entry);
	}

	std::optional<Node> PrimaryBuiltInCallExpression::result() const {
		return built_in_call_->result();
	}

	std::unique_ptr<Expression> PrimaryBuiltInCallExpression::clone() const {
		return std::make_unique<PrimaryBuiltInCallExpression>(built_in_call_->clone());
	}

	std::vector<Variable> PrimaryBuiltInCallExpression::variables() const {
		return built_in_call_->variables();
	}

}// namespace Dice::sparql2tensor::expressions