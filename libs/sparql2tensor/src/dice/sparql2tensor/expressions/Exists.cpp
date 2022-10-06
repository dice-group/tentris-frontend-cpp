#include "Exists.hpp"

#include <robin_hood.h>
#include "dice/rdf-tensor/Query.hpp"

namespace dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;


	Exists::Exists(std::vector<rdf4cpp::rdf::query::Variable> variables,
				   boost::container::flat_map<char, size_t> var_ids_positions,
				   rdf_tensor::Query sub_query,
				   bool not_exists,
				   std::chrono::steady_clock::time_point timeout)
		: sub_query_(std::move(sub_query)), not_exists_(not_exists),
		  timeout_(timeout), variables_(std::move(variables)), var_ids_positions_(std::move(var_ids_positions)) {}

	void Exists::update_value(const rdf_tensor::Entry &entry) {
		for (auto const &[var_id, pos] : var_ids_positions_) {
			sub_query_.assign_value_to_var(var_id, entry[pos]);
		}
	}

	rdf_tensor::NodeWrapper Exists::evaluate() const {
		auto generator_iter = rdf_tensor::QueryEvaluation::evaluate(sub_query_, timeout_);
		bool has_solutions = false;
		if (generator_iter.begin() != generator_iter.end() and (*generator_iter.begin()).value() > 0) {
			has_solutions = true;
		}
		bool result = not_exists_ ? not has_solutions : has_solutions;
		if (result) return TrueLiteral::instance();
		return FalseLiteral::instance();
	}

	std::vector<rdf4cpp::rdf::query::Variable> Exists::variables() const {
		return variables_;
	}

	Exists *Exists::clone_impl() const {
		return new Exists(variables_, var_ids_positions_, sub_query_, not_exists_, timeout_);
	}


}// namespace dice::sparql2tensor::expressions