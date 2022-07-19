#ifndef DICE_SPARQL_AGGREGATES_HPP
#define DICE_SPARQL_AGGREGATES_HPP

#include "Expression.hpp"

namespace Dice::sparql2tensor::expressions {

	/* https://www.w3.org/TR/sparql11-query/#rAggregate */
	class Aggregate : public SPARQLExpression {
	protected:
		std::unique_ptr<SPARQLExpression> op_expr_;
	public:
		explicit Aggregate(std::unique_ptr<SPARQLExpression> op_expr);
		~Aggregate() override = default;
		void update_value(rdf_tensor::Entry const &entry) override = 0;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override = 0;
		[[nodiscard]] std::unique_ptr<SPARQLExpression> clone_sparql() const override = 0;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	};

	class CountStar : public Aggregate {
	private:
		size_t count_ = 0;
	public:
		explicit CountStar(size_t count = 0);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::unique_ptr<SPARQLExpression> clone_sparql() const override;
	};

	class CountStarDistinct : public Aggregate {
	private:
		std::set<rdf_tensor::Entry> entries_;
	public:
		explicit CountStarDistinct(std::set<rdf_tensor::Entry> entries = {});
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::unique_ptr<SPARQLExpression> clone_sparql() const override;
	};

	class Count : public Aggregate {
	private:
		size_t count_ = 0;
	public:
		explicit Count(std::unique_ptr<SPARQLExpression> expr, size_t count = 0);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::unique_ptr<SPARQLExpression> clone_sparql() const override;
	};

	class CountDistinct : public Aggregate {
	private:
		std::set<rdf4cpp::rdf::Node> rdf_nodes_;
	public:
		explicit CountDistinct(std::unique_ptr<SPARQLExpression> expr, std::set<rdf4cpp::rdf::Node> rdf_nodes = {});
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::unique_ptr<SPARQLExpression> clone_sparql() const override;
	};

}// namespace Dice::sparql2tensor::expressions

#endif//DICE_SPARQL_AGGREGATES_HPP
