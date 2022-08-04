#include "Exists.hpp"

namespace dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;


	Exists::Exists(std::vector<std::unique_ptr<PrimaryVarExpression>> vars, rdf_tensor::Query sub_query, bool not_exists)
		: vars_in_scope_(std::move(vars)), sub_query_(std::move(sub_query)), not_exists_(not_exists) {}

	void Exists::update_value(const rdf_tensor::Entry &entry) {
		for (auto const &var_expr : vars_in_scope_) {
			var_expr->update_value(entry);
		}
	}

	rdf_tensor::NodeWrapper Exists::evaluate() const {
		std::vector<std::pair<char, rdf_tensor::NodeWrapper>> evaluated_vars{};
		for (auto const &var_expr : vars_in_scope_) {
			evaluated_vars.emplace_back(var_expr->query_level_var_id(), var_expr->evaluate());
		}
		auto generator_iter = rdf_tensor::QueryEvaluation::evaluate(sub_query_, std::chrono::steady_clock::time_point::max(), evaluated_vars);
		bool has_solutions = false;
		if (generator_iter.begin() != generator_iter.end() and (*generator_iter.begin()).value() > 0)
			has_solutions = true;
		bool result = not_exists_ ? not has_solutions : has_solutions;
		return Literal{std::to_string(result), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#boolean")};
	}

	std::vector<rdf4cpp::rdf::query::Variable> Exists::variables() const {
		std::vector<rdf4cpp::rdf::query::Variable> variables{};
		for (auto const &var_expr : vars_in_scope_) {
			auto const &vars = var_expr->variables();
			variables.insert(variables.end(), vars.begin(), vars.end());
		}
		return variables;
	}

	Exists *Exists::clone_impl() const {
		std::vector<std::unique_ptr<PrimaryVarExpression>> vars_copy{};
		for (auto const &var : vars_in_scope_) {
			auto *raw_var_expr_ptr = dynamic_cast<PrimaryVarExpression *>(var->clone().release());
			vars_copy.push_back(std::make_unique<PrimaryVarExpression>(*raw_var_expr_ptr));
		}
		return new Exists(std::move(vars_copy), sub_query_, not_exists_);
	}


}// namespace dice::sparql2tensor::expressions