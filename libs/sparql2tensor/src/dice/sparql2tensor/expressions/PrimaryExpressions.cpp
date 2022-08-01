#include "PrimaryExpressions.hpp"

namespace dice::sparql2tensor::expressions {

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

	PrimaryVarExpression *PrimaryVarExpression::clone_impl() const {
		return new PrimaryVarExpression(*this);
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

	PrimaryLiteralExpression *PrimaryLiteralExpression::clone_impl() const {
		return new PrimaryLiteralExpression(*this);
	}

	std::vector<Variable> PrimaryLiteralExpression::variables() const {
		return {};
	}

	/* IRI Expression */
	PrimaryIRIExpression::PrimaryIRIExpression(IRI iri)
		: iri_(iri) {}

	void PrimaryIRIExpression::update_value([[maybe_unused]] rdf_tensor::Entry const &entry) {}

	rdf_tensor::NodeWrapper PrimaryIRIExpression::evaluate() const {
		return iri_;
	}

	PrimaryIRIExpression *PrimaryIRIExpression::clone_impl() const {
		return new PrimaryIRIExpression(*this);
	}

	std::vector<Variable> PrimaryIRIExpression::variables() const {
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

	PrimaryBuiltInCallExpression *PrimaryBuiltInCallExpression::clone_impl() const {
		return new PrimaryBuiltInCallExpression(built_in_call_->clone());
	}

	std::vector<Variable> PrimaryBuiltInCallExpression::variables() const {
		return built_in_call_->variables();
	}

}// namespace dice::sparql2tensor::expressions