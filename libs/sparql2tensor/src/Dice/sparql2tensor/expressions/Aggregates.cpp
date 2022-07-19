#include "Aggregates.hpp"

#include <utility>

namespace Dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	/* Aggregate Exression */
	Aggregate::Aggregate(std::unique_ptr<SPARQLExpression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	[[nodiscard]] std::vector<Variable> Aggregate::variables() const {
		return op_expr_->variables();
	}

	/* CountStar Expression */
	CountStar::CountStar(size_t count)
		: Aggregate(nullptr), count_(count) {}

	void CountStar::update_value([[maybe_unused]] rdf_tensor::Entry const &key) {
		count_++;
	}

	rdf_tensor::NodeWrapper CountStar::evaluate() const {
		return Literal(std::to_string(count_), IRI("http://www.w3.org/2001/XMLSchema#integer"));
	}

	std::unique_ptr<SPARQLExpression> CountStar::clone_sparql() const {
		return std::make_unique<CountStar>(count_);
	}

	[[nodiscard]] std::vector<Variable> CountStar::variables() const {
		return {};
	}

	/* CountStarDistinct Expression */
	CountStarDistinct::CountStarDistinct(std::set<rdf_tensor::Entry> entries)
		: Aggregate(nullptr), entries_(std::move(entries)) {}

	void CountStarDistinct::update_value(rdf_tensor::Entry const &entry) {
		entries_.insert(entry);
	}

	rdf_tensor::NodeWrapper CountStarDistinct::evaluate() const {
		return Literal(std::to_string(entries_.size()), IRI("http://www.w3.org/2001/XMLSchema#integer"));
	}

	std::unique_ptr<SPARQLExpression> CountStarDistinct::clone_sparql() const {
		return std::make_unique<CountStarDistinct>(entries_);
	}

	[[nodiscard]] std::vector<Variable> CountStarDistinct::variables() const {
		return {};
	}

	/* Count Expression */
	Count::Count(std::unique_ptr<SPARQLExpression> expr, size_t count)
		: Aggregate(std::move(expr)), count_(count) {}

	void Count::update_value(rdf_tensor::Entry const &entry) {
		op_expr_->update_value(entry);
		auto expr_res = op_expr_->evaluate();
//		if (expr_res.has_value())
		count_++;
	}

	rdf_tensor::NodeWrapper Count::evaluate() const {
		return Literal(std::to_string(count_), IRI("http://www.w3.org/2001/XMLSchema#integer"));
	}

	std::unique_ptr<SPARQLExpression> Count::clone_sparql() const {
		return std::make_unique<Count>(op_expr_->clone_sparql(), count_);
	}

	/* CountDistinct Expression */
	CountDistinct::CountDistinct(std::unique_ptr<SPARQLExpression> expr, std::set<Node> rdf_nodes)
		: Aggregate(std::move(expr)), rdf_nodes_(std::move(rdf_nodes)) {}

	void CountDistinct::update_value([[maybe_unused]] rdf_tensor::Entry const &entry) {
		op_expr_->update_value(entry);
		auto expr_res = op_expr_->evaluate();
//		if (expr_res.has_value())
		rdf_nodes_.insert(expr_res);
	}

	rdf_tensor::NodeWrapper CountDistinct::evaluate() const {
		return Literal(std::to_string(rdf_nodes_.size()), IRI("http://www.w3.org/2001/XMLSchema#integer"));
	}

	std::unique_ptr<SPARQLExpression> CountDistinct::clone_sparql() const {
		return std::make_unique<CountDistinct>(op_expr_->clone_sparql(), rdf_nodes_);
	}


}// namespace Dice::sparql2tensor::expressions