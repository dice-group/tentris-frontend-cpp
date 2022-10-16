#include "SPARQLQuery.hpp"


namespace dice::sparql2tensor {

	SPARQLQuery::SPARQLQuery(rdf_tensor::HypertrieContext_ptr context)
		: raw_query_(context) {}

	rdf_tensor::Query SPARQLQuery::raw_query() const { return raw_query_; }

	const std::vector<rdf4cpp::rdf::query::Variable> &SPARQLQuery::projected_variables() const { return projected_variables_; }

	bool SPARQLQuery::ask() const { return ask_; }

	void SPARQLQuery::set_ask() { ask_ = true; }

	void SPARQLQuery::register_variable(rdf4cpp::rdf::query::Variable var) {
		if (var_to_id_.contains(var))
			return;
		var_to_id_[var] = next_var_id++;
	}

	void SPARQLQuery::add_projected_variable(rdf4cpp::rdf::query::Variable var) {
		projected_variables_.push_back(var);
	}

	rdf_tensor::operand_desc SPARQLQuery::add_triple_pattern(const rdf4cpp::rdf::query::TriplePattern &tp,
															 const triple_store::TripleStore &triple_store) {
		std::vector<char> vars_ids{};
		for (auto const &node : tp) {
			if (not node.is_variable())
				continue;
			vars_ids.push_back(var_to_id_[rdf4cpp::rdf::query::Variable(node)]);
		}
		rdf_tensor::SliceKey slice_key;
		slice_key.reserve(3);
		for (auto const &node : tp) {
			if (node.is_variable())
				slice_key.push_back(std::nullopt);
			else
				slice_key.push_back(node);
		}
		auto slice_result = triple_store.get_hypertrie()[slice_key];
		if (slice_key.get_fixed_depth() == 3) {
			auto entry_exists = std::get<bool>(slice_result);
			if (entry_exists)
				return raw_query_.add_operand(vars_ids, triple_store.get_true_scalar());
			return raw_query_.add_operand(vars_ids, triple_store.get_false_scalar());
		}
		return raw_query_.add_operand(vars_ids, std::get<rdf_tensor::const_BoolHypertrie>(slice_result));
	}

	rdf_tensor::operand_desc SPARQLQuery::add_inline_data(const rdf4cpp::rdf::query::Variable &var,
														  const std::vector<rdf4cpp::rdf::Node> &data,
														  const triple_store::TripleStore &triple_store) {
		rdf_tensor::BoolHypertrie data_ht{1, raw_query_.context()};
		for (auto const &node : data) {
			data_ht.set({node}, true);
		}
		return raw_query_.add_operand({var_to_id_[var]}, std::move(data_ht));
	}

	rdf_tensor::operand_desc SPARQLQuery::add_filter_expr(std::unique_ptr<expressions::SPARQLExpression> expression,
														  const triple_store::TripleStore &triple_store) {
		auto variables = expression->variables();
		std::vector<char> vars_ids{};
		for (auto var : variables) {
			assert(var_to_id_.contains(var));
			vars_ids.push_back(var_to_id_[var]);
		}
		return raw_query_.add_filter(vars_ids, std::move(expression), triple_store.get_true_scalar());
	}

	rdf_tensor::operand_desc SPARQLQuery::add_subquery(SPARQLQuery subquery) {
		std::vector<char> vars_ids{};
		for (auto var : subquery.projected_variables()) {
			assert(var_to_id_.contains(var));
			vars_ids.push_back(var_to_id_[var]);
		}
		return raw_query_.add_subquery(vars_ids, subquery.raw_query());
	}

	void SPARQLQuery::add_dependency(rdf_tensor::operand_desc operand_1, rdf_tensor::operand_desc operand_2, bool bidirectional) {
		raw_query_.add_dependency(operand_1, operand_2, bidirectional);
	}

	void SPARQLQuery::add_connection(rdf_tensor::operand_desc operand_1, rdf_tensor::operand_desc operand_2, bool bidirectional) {
		raw_query_.add_connection(operand_1, operand_2, bidirectional);
	}

	void SPARQLQuery::track_variable(rdf4cpp::rdf::query::Variable variable) {
		assert(var_to_id_.contains(variable));
		raw_query_.track_variable(var_to_id_[variable]);
	}

	size_t SPARQLQuery::tracked_variable_position(rdf4cpp::rdf::query::Variable variable) {
		assert(var_to_id_.contains(variable));
		return raw_query_.tracked_var_position(var_to_id_[variable]);
	}

	char SPARQLQuery::variable_id(rdf4cpp::rdf::query::Variable variable) {
		assert(var_to_id_.contains(variable));
		return var_to_id_[variable];
	}

	void SPARQLQuery::add_solution_binding(std::unique_ptr<expressions::SPARQLExpression> expression) {
		raw_query_.add_binding(std::move(expression));
	}

	void SPARQLQuery::add_grouping_expression(std::unique_ptr<expressions::SPARQLExpression> expression) {
		raw_query_.add_grouping_expression(std::move(expression));
	}

	void SPARQLQuery::assign_value_to_var(rdf4cpp::rdf::query::Variable var, rdf_tensor::NodeWrapper value) {
		raw_query_.assign_value_to_var(var_to_id_[var], value);
	}

	void SPARQLQuery::set_distinct() { raw_query_.set_distinct(); }

	void SPARQLQuery::set_aggregates() { raw_query_.set_aggregates(); }

	bool SPARQLQuery::contains_aggregates() const { return raw_query_.contains_aggregates(); }

}// namespace dice::sparql2tensor
