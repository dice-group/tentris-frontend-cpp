#include "PrimaryExpressions.hpp"

namespace Dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	/* Variable Expression */
	PrimaryVarExpression::PrimaryVarExpression(Variable variable, size_t var_pos_in_entry)
		: var_pos_in_entry_(var_pos_in_entry), rdf_node_(), variable_(variable) {}

	void PrimaryVarExpression::update_value(rdf_tensor::Entry const &entry) {
		rdf_node_ = entry[var_pos_in_entry_];
	}

	rdf_tensor::NodeWrapper PrimaryVarExpression::evaluate() const {
		return rdf_node_;
	}

	std::unique_ptr<SPARQLExpression> PrimaryVarExpression::clone_sparql() const {
		return std::make_unique<PrimaryVarExpression>(*this);
	}

	std::vector<Variable> PrimaryVarExpression::variables() const {
		return {variable_};
	}

	/* Literal Expression */
	PrimaryLiteralExpression::PrimaryLiteralExpression(Literal literal)
		: literal_(literal) {}

	void PrimaryLiteralExpression::update_value([[maybe_unused]] rdf_tensor::Entry const &entry) {}

	rdf_tensor::NodeWrapper PrimaryLiteralExpression::evaluate() const {
		return literal_;
	}

	std::unique_ptr<SPARQLExpression> PrimaryLiteralExpression::clone_sparql() const {
		return std::make_unique<PrimaryLiteralExpression>(*this);
	}

	std::vector<Variable> PrimaryLiteralExpression::variables() const {
		return {};
	}

	/* BuiltInCall Expression */
	PrimaryBuiltInCallExpression::PrimaryBuiltInCallExpression(std::unique_ptr<SPARQLExpression> expr)
		: built_in_call_(std::move(expr)) {}

	void PrimaryBuiltInCallExpression::update_value(rdf_tensor::Entry const &entry) {
		return built_in_call_->update_value(entry);
	}

	rdf_tensor::NodeWrapper PrimaryBuiltInCallExpression::evaluate() const {
		return built_in_call_->evaluate();
	}

	std::unique_ptr<SPARQLExpression> PrimaryBuiltInCallExpression::clone_sparql() const {
		return std::make_unique<PrimaryBuiltInCallExpression>(built_in_call_->clone_sparql());
	}

	std::vector<Variable> PrimaryBuiltInCallExpression::variables() const {
		return built_in_call_->variables();
	}

}// namespace Dice::sparql2tensor::expressions