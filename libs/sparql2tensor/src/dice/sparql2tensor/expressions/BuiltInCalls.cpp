#include "BuiltInCalls.hpp"

namespace dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	bool compatible_str_arguments(rdf4cpp::rdf::Literal str_1, rdf4cpp::rdf::Literal str_2) {
		auto str_datatype = "http://www.w3.org/2001/XMLSchema#string";
		auto langstring_datatype = "http://www.w3.org/1999/02/22-rdf-syntax-ns#langString";
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
		return {Literal(expr_result.backend_handle().iri_backend().identifier)};
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

	/* StrStarts Expression */
	StrStarts::StrStarts(std::unique_ptr<SPARQLExpression> op_expr_1, std::unique_ptr<SPARQLExpression> op_expr_2)
		: op_expr_1_(std::move(op_expr_1)), op_expr_2_(std::move(op_expr_2)) {}

	void StrStarts::update_value(const rdf_tensor::Entry &entry) {
		op_expr_1_->update_value(entry);
		op_expr_2_->update_value(entry);
	}

	rdf_tensor::NodeWrapper StrStarts::evaluate() const {
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
		if (literal_1.lexical_form().starts_with(literal_2.lexical_form()))
			return Literal("true", IRI("http://www.w3.org/2001/XMLSchema#boolean"));
		return Literal("false", IRI("http://www.w3.org/2001/XMLSchema#boolean"));
	}

	StrStarts *StrStarts::clone_impl() const {
		return new StrStarts(op_expr_1_->clone(), op_expr_2_->clone());
	}

	std::vector<Variable> StrStarts::variables() const {
		auto vars_1 = op_expr_1_->variables();
		auto vars_2 = op_expr_2_->variables();
		vars_1.insert(vars_1.end(), vars_2.begin(), vars_2.end());
		return vars_1;
	}

	/* StrEnds Expression */
	StrEnds::StrEnds(std::unique_ptr<SPARQLExpression> op_expr_1, std::unique_ptr<SPARQLExpression> op_expr_2)
		: op_expr_1_(std::move(op_expr_1)), op_expr_2_(std::move(op_expr_2)) {}

	void StrEnds::update_value(const rdf_tensor::Entry &entry) {
		op_expr_1_->update_value(entry);
		op_expr_2_->update_value(entry);
	}

	rdf_tensor::NodeWrapper StrEnds::evaluate() const {
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
		if (literal_1.lexical_form().ends_with(literal_2.lexical_form()))
			return Literal("true", IRI("http://www.w3.org/2001/XMLSchema#boolean"));
		return Literal("false", IRI("http://www.w3.org/2001/XMLSchema#boolean"));
	}

	StrEnds *StrEnds::clone_impl() const {
		return new StrEnds(op_expr_1_->clone(), op_expr_2_->clone());
	}

	std::vector<Variable> StrEnds::variables() const {
		auto vars_1 = op_expr_1_->variables();
		auto vars_2 = op_expr_2_->variables();
		vars_1.insert(vars_1.end(), vars_2.begin(), vars_2.end());
		return vars_1;
	}

	/* LangMatches Expression */
	LangMatches::LangMatches(std::unique_ptr<SPARQLExpression> op_expr_1, std::unique_ptr<SPARQLExpression> op_expr_2)
		: op_expr_1_(std::move(op_expr_1)), op_expr_2_(std::move(op_expr_2)) {}

	void LangMatches::update_value(const rdf_tensor::Entry &entry) {
		op_expr_1_->update_value(entry);
		op_expr_2_->update_value(entry);
	}

	rdf_tensor::NodeWrapper LangMatches::evaluate() const {
		auto expr_result_1 = op_expr_1_->evaluate();
		auto expr_result_2 = op_expr_2_->evaluate();
		if (expr_result_1.null() or expr_result_2.null())
			return {};
		if (not(expr_result_1.is_literal() and expr_result_2.is_literal()))
			return {};
		auto literal_1 = Literal(expr_result_1);
		auto literal_2 = Literal(expr_result_2);
		auto literal_1_str = literal_1.lexical_form();
		auto literal_2_str = literal_2.lexical_form();
		if (literal_1.datatype() != IRI("http://www.w3.org/2001/XMLSchema#string") or
			literal_2.datatype() != IRI("http://www.w3.org/2001/XMLSchema#string"))
			return {};
		if (literal_2_str == "*" and not literal_1_str.empty())
			return Literal("true", IRI("http://www.w3.org/2001/XMLSchema#boolean"));
		// case-insensitive comparison of language tags (https://stackoverflow.com/a/28387449)
		if (literal_1_str.length() == literal_2_str.length() and
			std::equal(literal_1.language_tag().begin(), literal_1.language_tag().end(), literal_2.lexical_form().begin(),
					   [](char const &a, char const &b) { return std::tolower(a) == std::tolower(b); }))
			return Literal("true", IRI("http://www.w3.org/2001/XMLSchema#boolean"));
		return Literal("false", IRI("http://www.w3.org/2001/XMLSchema#boolean"));
	}

	LangMatches *LangMatches::clone_impl() const {
		return new LangMatches(op_expr_1_->clone(), op_expr_2_->clone());
	}

	std::vector<Variable> LangMatches::variables() const {
		auto vars_1 = op_expr_1_->variables();
		auto vars_2 = op_expr_2_->variables();
		vars_1.insert(vars_1.end(), vars_2.begin(), vars_2.end());
		return vars_1;
	}

	/* Lang Expression */
	Lang::Lang(std::unique_ptr<SPARQLExpression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	void Lang::update_value(const rdf_tensor::Entry &entry) {
		op_expr_->update_value(entry);
	}

	rdf_tensor::NodeWrapper Lang::evaluate() const {
		auto expr_result = op_expr_->evaluate();
		if (expr_result.null() or not expr_result.is_literal())
			return {};
		auto lang_tag = Literal(expr_result).language_tag();
		if (lang_tag.empty())
			return Literal("");
		return Literal(lang_tag);
	}

	Lang *Lang::clone_impl() const {
		return new Lang(op_expr_->clone());
	}

	std::vector<Variable> Lang::variables() const {
		return op_expr_->variables();
	}

	/* StrLang Expression */
	StrLang::StrLang(std::unique_ptr<SPARQLExpression> op_expr_1, std::unique_ptr<SPARQLExpression> op_expr_2)
		: op_expr_1_(std::move(op_expr_1)), op_expr_2_(std::move(op_expr_2)) {}

	void StrLang::update_value(const rdf_tensor::Entry &entry) {
		op_expr_1_->update_value(entry);
		op_expr_2_->update_value(entry);
	}

	rdf_tensor::NodeWrapper StrLang::evaluate() const {
		auto expr_result_1 = op_expr_1_->evaluate();
		auto expr_result_2 = op_expr_2_->evaluate();
		if (expr_result_1.null() or expr_result_2.null())
			return {};
		if (not(expr_result_1.is_literal() and expr_result_2.is_literal()))
			return {};
		auto literal_1 = Literal(expr_result_1);
		auto literal_2 = Literal(expr_result_2);
		if (literal_1.datatype() != IRI("http://www.w3.org/2001/XMLSchema#string") or
			literal_2.datatype() != IRI("http://www.w3.org/2001/XMLSchema#string"))
			return {};
		return Literal(literal_1.lexical_form(), literal_2.lexical_form());
	}

	StrLang *StrLang::clone_impl() const {
		return new StrLang(op_expr_1_->clone(), op_expr_2_->clone());
	}

	std::vector<Variable> StrLang::variables() const {
		auto vars_1 = op_expr_1_->variables();
		auto vars_2 = op_expr_2_->variables();
		vars_1.insert(vars_1.end(), vars_2.begin(), vars_2.end());
		return vars_1;
	}

}// namespace dice::sparql2tensor::expressions