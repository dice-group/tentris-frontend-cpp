#include "Aggregates.hpp"

#include <utility>

namespace Dice::sparql2tensor::expressions {

	/* Aggregate Exression */
	Aggregate::Aggregate(std::unique_ptr<Expression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> Aggregate::variables() const {
		return op_expr_->variables();
	}

	/* CountStar Expression */
	CountStar::CountStar(size_t count)
		: Aggregate(nullptr), count_(count) {}

	void CountStar::evaluate([[maybe_unused]] const rdf_tensor::Entry &entry) {
		count_++;
	}

	rdf4cpp::rdf::Node CountStar::result() const {
		return rdf4cpp::rdf::Literal(std::to_string(count_), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"));
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

	rdf4cpp::rdf::Node CountStarDistinct::result() const {
		return rdf4cpp::rdf::Literal(std::to_string(entries_.size()), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"));
	}

	std::unique_ptr<Expression> CountStarDistinct::clone() const {
		return std::make_unique<CountStarDistinct>(entries_);
	}

	/* Count Expression */
	Count::Count(std::unique_ptr<Expression> expr, size_t count)
		: Aggregate(std::move(expr)), count_(count) {}

	void Count::evaluate([[maybe_unused]] const rdf_tensor::Entry &entry) {
		count_++;
	}

	rdf4cpp::rdf::Node Count::result() const {
		return rdf4cpp::rdf::Literal(std::to_string(count_), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"));
	}

	std::unique_ptr<Expression> Count::clone() const {
		return std::make_unique<Count>(op_expr_->clone(), count_);
	}

	/* CountDistinct Expression */
	CountDistinct::CountDistinct(std::unique_ptr<Expression> expr, std::set<rdf4cpp::rdf::Node> rdf_nodes)
		: Aggregate(std::move(expr)), rdf_nodes_(std::move(rdf_nodes)) {}

	void CountDistinct::evaluate([[maybe_unused]] const rdf_tensor::Entry &entry) {
		op_expr_->evaluate(entry);
		rdf_nodes_.insert(op_expr_->result());
	}

	rdf4cpp::rdf::Node CountDistinct::result() const {
		return rdf4cpp::rdf::Literal(std::to_string(rdf_nodes_.size()), rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"));
	}

	std::unique_ptr<Expression> CountDistinct::clone() const {
		return std::make_unique<CountDistinct>(op_expr_->clone(), rdf_nodes_);
	}


}// namespace Dice::sparql2tensor::expressions