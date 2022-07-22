#include "BuiltInCalls.hpp"

namespace Dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	bool compatible_str_arguments(rdf4cpp::rdf::Literal str_1, rdf4cpp::rdf::Literal str_2) {
		auto str_datatype = "http://www.w3.org/2001/XMLSchema#string";
		auto langstring_datatype = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
		if (str_1.datatype().identifier() == str_datatype and str_2.datatype().identifier() == str_datatype)
			return true;
		if ((str_1.datatype().identifier() == langstring_datatype and str_2.datatype().identifier() == langstring_datatype) and
				 (str_1.language_tag() == str_2.language_tag()))
			return true;
		if (str_1.datatype().identifier() == langstring_datatype and str_2.datatype().identifier() == str_datatype)
			return true;
		return false;
	}

	/* IsIRI Expression */
	IsIRI::IsIRI(std::unique_ptr<SPARQLExpression> op_expr)
		: op_expr_(std::move(op_expr)) {}


	void IsIRI::update_value(const rdf_tensor::Entry &entry) {
		op_expr_->update_value(entry);
	}

	rdf_tensor::NodeWrapper IsIRI::evaluate() const {
		auto expr_result = op_expr_->evaluate();
		if (expr_result.null())
			return {};
		return Literal(std::to_string(expr_result.is_iri()),
					   IRI(IRI("http://www.w3.org/2001/XMLSchema#boolean")));
	}

	IsIRI *IsIRI::clone_impl() const {
		return new IsIRI(op_expr_->clone());
	}

	std::vector<Variable> IsIRI::variables() const {
		return op_expr_->variables();
	}

	/* IsBlank Expression */
	IsBlank::IsBlank(std::unique_ptr<SPARQLExpression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	void IsBlank::update_value(const rdf_tensor::Entry &entry) {
		op_expr_->update_value(entry);
	}

	rdf_tensor::NodeWrapper IsBlank::evaluate() const {
		auto expr_result = op_expr_->evaluate();
		if (expr_result.null())
			return {};
		return Literal(std::to_string(expr_result.is_blank_node()),
					   IRI(IRI("http://www.w3.org/2001/XMLSchema#boolean")));
	}

	IsBlank *IsBlank::clone_impl() const {
		return new IsBlank(op_expr_->clone());
	}

	std::vector<Variable> IsBlank::variables() const {
		return op_expr_->variables();
	}

	/* IsLiteral Expression */
	IsLiteral::IsLiteral(std::unique_ptr<SPARQLExpression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	void IsLiteral::update_value(const rdf_tensor::Entry &entry) {
		op_expr_->update_value(entry);
	}

	rdf_tensor::NodeWrapper IsLiteral::evaluate() const {
		auto expr_result = op_expr_->evaluate();
		if (expr_result.null())
			return {};
		return Literal(std::to_string(expr_result.is_literal()),
					   IRI(IRI("http://www.w3.org/2001/XMLSchema#boolean")));
	}

	IsLiteral *IsLiteral::clone_impl() const {
		return new IsLiteral(op_expr_->clone());
	}

	std::vector<Variable> IsLiteral::variables() const {
		return op_expr_->variables();
	}

	/* Datatype Expression */
	Datatype::Datatype(std::unique_ptr<SPARQLExpression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	void Datatype::update_value(const rdf_tensor::Entry &entry) {
		op_expr_->update_value(entry);
	}

	rdf_tensor::NodeWrapper Datatype::evaluate() const {
		auto expr_result = op_expr_->evaluate();
		if (expr_result.null() or not expr_result.is_literal())
			return {};
		return static_cast<Literal>(expr_result).datatype();
	}

	Datatype *Datatype::clone_impl() const {
		return new Datatype(op_expr_->clone());
	}

	std::vector<Variable> Datatype::variables() const {
		return op_expr_->variables();
	}

	/* Str Expression */
	Str::Str(std::unique_ptr<SPARQLExpression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	void Str::update_value(const rdf_tensor::Entry &entry) {
		op_expr_->update_value(entry);
	}

	rdf_tensor::NodeWrapper Str::evaluate() const {
		auto expr_result = op_expr_->evaluate();
		if (expr_result.null() or (not expr_result.is_literal() and not expr_result.is_iri()))
			return {};
		// literal case
		if (expr_result.is_literal()) {
			return {Literal(expr_result.backend_handle().literal_backend().lexical_form)};
		}
		// iri case
		return {Literal(std::string(expr_result))};
	}

	Str *Str::clone_impl() const {
		return new Str(op_expr_->clone());
	}

	std::vector<Variable> Str::variables() const {
		return op_expr_->variables();
	}

	/* Contains Expression */
	Contains::Contains(std::unique_ptr<SPARQLExpression> op_expr_1, std::unique_ptr<SPARQLExpression> op_expr_2)
		: op_expr_1_(std::move(op_expr_1)), op_expr_2_(std::move(op_expr_2)) {}

	void Contains::update_value(const rdf_tensor::Entry &entry) {
		op_expr_1_->update_value(entry);
		op_expr_2_->update_value(entry);
	}

	rdf_tensor::NodeWrapper Contains::evaluate() const {
		auto expr_result_1 = op_expr_1_->evaluate();
		auto expr_result_2 = op_expr_2_->evaluate();
		if (expr_result_1.null() or expr_result_2.null())
			return {};
		if (not(expr_result_1.is_literal() and expr_result_2.is_literal()))
			return {};
		auto literal_1 = Literal(expr_result_1);
		auto literal_2 = Literal(expr_result_2);
		if (not compatible_str_arguments(literal_1, literal_2))
			return {};
		if (literal_1.lexical_form().find(literal_2.lexical_form()) != std::string::npos)
			return Literal("true", IRI("http://www.w3.org/2001/XMLSchema#boolean"));
		return Literal("false", IRI("http://www.w3.org/2001/XMLSchema#boolean"));
	}

	Contains *Contains::clone_impl() const {
		return new Contains(op_expr_1_->clone(), op_expr_2_->clone());
	}

	std::vector<Variable> Contains::variables() const {
		auto vars_1 = op_expr_1_->variables();
		auto vars_2 = op_expr_2_->variables();
		vars_1.insert(vars_1.end(), vars_2.begin(), vars_2.end());
		return vars_1;
	}

}// namespace Dice::sparql2tensor::expressions