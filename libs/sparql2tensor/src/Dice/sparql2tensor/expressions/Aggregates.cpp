#include "Aggregates.hpp"

#include <utility>

namespace Dice::sparql2tensor::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	/* Aggregate Exression */
	Aggregate::Aggregate(std::unique_ptr<Expression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	[[nodiscard]] std::vector<Variable> Aggregate::variables() const {
		return op_expr_->variables();
	}

	/* CountStar Expression */
	CountStar::CountStar(size_t count)
		: Aggregate(nullptr), count_(count) {}

	void CountStar::evaluate([[maybe_unused]] const rdf_tensor::Entry &entry) {
		count_++;
	}

	std::optional<Node> CountStar::result() const {
		return Literal(std::to_string(count_), IRI("http://www.w3.org/2001/XMLSchema#integer"));
	}

	std::unique_ptr<Expression> CountStar::clone() const {
		return std::make_unique<CountStar>(count_);
	}

	/* CountStarDistinct Expression */
	CountStarDistinct::CountStarDistinct(std::set<rdf_tensor::Entry> entries)
		: Aggregate(nullptr), entries_(std::move(entries)) {}

	void CountStarDistinct::evaluate(const rdf_tensor::Entry &entry) {
		entries_.insert(entry);
	}

	std::optional<Node> CountStarDistinct::result() const {
		return Literal(std::to_string(entries_.size()), IRI("http://www.w3.org/2001/XMLSchema#integer"));
	}

	std::unique_ptr<Expression> CountStarDistinct::clone() const {
		return std::make_unique<CountStarDistinct>(entries_);
	}

	/* Count Expression */
	Count::Count(std::unique_ptr<Expression> expr, size_t count)
		: Aggregate(std::move(expr)), count_(count) {}

	void Count::evaluate(const rdf_tensor::Entry &entry) {
		op_expr_->evaluate(entry);
		auto expr_res = op_expr_->result();
		if (expr_res.has_value())
			count_++;
	}

	std::optional<Node> Count::result() const {
		return Literal(std::to_string(count_), IRI("http://www.w3.org/2001/XMLSchema#integer"));
	}

	std::unique_ptr<Expression> Count::clone() const {
		return std::make_unique<Count>(op_expr_->clone(), count_);
	}

	/* CountDistinct Expression */
	CountDistinct::CountDistinct(std::unique_ptr<Expression> expr, std::set<Node> rdf_nodes)
		: Aggregate(std::move(expr)), rdf_nodes_(std::move(rdf_nodes)) {}

	void CountDistinct::evaluate([[maybe_unused]] const rdf_tensor::Entry &entry) {
		op_expr_->evaluate(entry);
		auto expr_res = op_expr_->result();
		if (expr_res.has_value())
			rdf_nodes_.insert(expr_res.value());
	}

	std::optional<Node> CountDistinct::result() const {
		return Literal(std::to_string(rdf_nodes_.size()), IRI("http://www.w3.org/2001/XMLSchema#integer"));
	}

	std::unique_ptr<Expression> CountDistinct::clone() const {
		return std::make_unique<CountDistinct>(op_expr_->clone(), rdf_nodes_);
	}


}// namespace Dice::sparql2tensor::expressions