#ifndef DICE_SPARQL_BUILTINCALLS_HPP
#define DICE_SPARQL_BUILTINCALLS_HPP

#include "Expression.hpp"

namespace dice::sparql2tensor::expressions {

	/* https://www.w3.org/TR/sparql11-query/#func-arg-compatibility */
	bool compatible_str_arguments(rdf4cpp::rdf::Literal str_1, rdf4cpp::rdf::Literal str_2);

	/* https://www.w3.org/TR/sparql11-query/#func-isiri */
	class IsIRI : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_;

	public:
		explicit IsIRI(std::unique_ptr<SPARQLExpression> op_expr);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] IsIRI *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-isblank */
	class IsBlank : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_;

	public:
		explicit IsBlank(std::unique_ptr<SPARQLExpression> op_expr);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] IsBlank *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-isliteral */
	class IsLiteral : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_;

	public:
		explicit IsLiteral(std::unique_ptr<SPARQLExpression> op_expr);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] IsLiteral *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-datatype */
	class Datatype : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_;

	public:
		explicit Datatype(std::unique_ptr<SPARQLExpression> op_expr);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] Datatype *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-str */
	class Str : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_;

	public:
		explicit Str(std::unique_ptr<SPARQLExpression> op_expr);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] Str *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-contains */
	class Contains : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_1_;
		std::unique_ptr<SPARQLExpression> op_expr_2_;

	public:
		explicit Contains(std::unique_ptr<SPARQLExpression> op_expr_1, std::unique_ptr<SPARQLExpression> op_expr_2);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] Contains *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-strstarts */
	class StrStarts : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_1_;
		std::unique_ptr<SPARQLExpression> op_expr_2_;

	public:
		explicit StrStarts(std::unique_ptr<SPARQLExpression> op_expr_1, std::unique_ptr<SPARQLExpression> op_expr_2);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] StrStarts *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-strends */
	class StrEnds : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_1_;
		std::unique_ptr<SPARQLExpression> op_expr_2_;

	public:
		explicit StrEnds(std::unique_ptr<SPARQLExpression> op_expr_1, std::unique_ptr<SPARQLExpression> op_expr_2);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] StrEnds *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-langMatches */
	class LangMatches : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_1_;
		std::unique_ptr<SPARQLExpression> op_expr_2_;

	public:
		explicit LangMatches(std::unique_ptr<SPARQLExpression> op_expr_1, std::unique_ptr<SPARQLExpression> op_expr_2);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] LangMatches *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-lang */
	class Lang : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_;

	public:
		explicit Lang(std::unique_ptr<SPARQLExpression> op_expr);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] Lang *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-strlang */
	class StrLang : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_1_;
		std::unique_ptr<SPARQLExpression> op_expr_2_;

	public:
		explicit StrLang(std::unique_ptr<SPARQLExpression> op_expr_1, std::unique_ptr<SPARQLExpression> op_expr_2);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] StrLang *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-bound */
	class Bound : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> op_expr_;

	public:
		explicit Bound(std::unique_ptr<SPARQLExpression> op_expr_);
		void update_value(rdf_tensor::Entry const &entry) override;
		[[nodiscard]] rdf_tensor::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] Bound *clone_impl() const override;
	};

}//namespace dice::sparql2tensor::expressions

#endif//DICE_SPARQL_BUILTINCALLS_HPP
