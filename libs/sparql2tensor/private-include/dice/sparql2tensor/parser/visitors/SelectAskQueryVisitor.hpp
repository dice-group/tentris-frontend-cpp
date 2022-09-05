#ifndef DICE_SPARQL_SELECTASKQUERYVISITOR_HPP
#define DICE_SPARQL_SELECTASKQUERYVISITOR_HPP

#include <rdf4cpp/rdf.hpp>

#include <SparqlParser/SparqlParserBaseVisitor.h>

#include "dice/sparql2tensor/SPARQLQuery.hpp"

#include <robin_hood.h>

namespace dice::sparql2tensor::parser::visitors {

	using namespace Dice::sparql_parser::base;

	class SelectAskQueryVisitor : public SparqlParserBaseVisitor {

	private:
		SPARQLQuery *const query;
		triple_store::TripleStore const &triple_store;
		robin_hood::unordered_map<std::string, std::string> prefixes;
		// for the construction of the raw query
		rdf4cpp::rdf::Node active_subject;
		rdf4cpp::rdf::Node active_predicate;
		std::unordered_set<rdf4cpp::rdf::query::Variable> vars_in_scope;
		std::unordered_set<rdf4cpp::rdf::query::Variable> vars_in_group_by;
		std::unordered_set<rdf4cpp::rdf::query::Variable> vars_in_select;
		// for the construction of the operand dependency graph
		std::vector<std::vector<uint8_t>> group_patterns;
		std::vector<std::vector<uint8_t>> opt_operands;
		std::vector<std::vector<uint8_t>> union_operands; // used to avoid connecting union patterns of the same optional pattern
		// for the "rewriting"
		std::vector<std::vector<SparqlParser::TriplesBlockContext *>> triples_blocks;
		std::vector<std::vector<SparqlParser::FilterContext *>> filter_blocks;
		std::vector<std::vector<SparqlParser::OptionalGraphPatternContext *>> optional_blocks;
		std::vector<std::vector<SparqlParser::SubSelectContext *>> subselect_blocks;

	public:
		SelectAskQueryVisitor() = delete;

		SelectAskQueryVisitor(SPARQLQuery *q,
							  triple_store::TripleStore const& ts,
							  robin_hood::unordered_map<std::string, std::string> prefixes);

		antlrcpp::Any visitAskQuery(SparqlParser::AskQueryContext *ctx) override;

		antlrcpp::Any visitSelectQuery(SparqlParser::SelectQueryContext *) override;

		antlrcpp::Any visitSelectClause(SparqlParser::SelectClauseContext *) override;

		antlrcpp::Any visitWhereClause(SparqlParser::WhereClauseContext *) override;

		antlrcpp::Any visitGroupGraphPattern(SparqlParser::GroupGraphPatternContext *) override;

		antlrcpp::Any visitSubSelect(SparqlParser::SubSelectContext *ctx) override;

		antlrcpp::Any visitGroupGraphPatternSub(SparqlParser::GroupGraphPatternSubContext *) override;

		antlrcpp::Any visitFilter(SparqlParser::FilterContext *ctx) override;

		antlrcpp::Any visitTriplesBlock(SparqlParser::TriplesBlockContext *) override;

		antlrcpp::Any visitTriplesSameSubjectPath(SparqlParser::TriplesSameSubjectPathContext *) override;

		antlrcpp::Any visitPropertyListPathNotEmpty(SparqlParser::PropertyListPathNotEmptyContext *) override;

		antlrcpp::Any visitVarOrTerm(SparqlParser::VarOrTermContext *) override;

		antlrcpp::Any visitIri(SparqlParser::IriContext *) override;

		antlrcpp::Any visitBlankNode(SparqlParser::BlankNodeContext *) override;

		antlrcpp::Any visitVar(SparqlParser::VarContext *) override;

		antlrcpp::Any visitObjectListPath(SparqlParser::ObjectListPathContext *) override;

		antlrcpp::Any visitObjectList(SparqlParser::ObjectListContext *) override;

		antlrcpp::Any visitObjectPath(SparqlParser::ObjectPathContext *) override;

		antlrcpp::Any visitObject(SparqlParser::ObjectContext *) override;

		antlrcpp::Any visitPath(SparqlParser::PathContext *) override;

		antlrcpp::Any visitPathAlternative(SparqlParser::PathAlternativeContext *) override;

		antlrcpp::Any visitPathSequence(SparqlParser::PathSequenceContext *) override;

		antlrcpp::Any visitPathEltOrInverse(SparqlParser::PathEltOrInverseContext *) override;

		antlrcpp::Any visitPathElt(SparqlParser::PathEltContext *) override;

		antlrcpp::Any visitGroupClause(SparqlParser::GroupClauseContext *ctx) override;

		antlrcpp::Any visitExpression(SparqlParser::ExpressionContext *ctx);

		antlrcpp::Any visitPrimaryExpression(SparqlParser::PrimaryExpressionContext *ctx) override;

		antlrcpp::Any visitConditionalAndExpression(SparqlParser::ConditionalAndExpressionContext *ctx) override;

		antlrcpp::Any visitConditionalOrExpression(SparqlParser::ConditionalOrExpressionContext *ctx) override;

		antlrcpp::Any visitRelationalExpression(SparqlParser::RelationalExpressionContext *ctx) override;

		antlrcpp::Any visitBuiltInCall(SparqlParser::BuiltInCallContext *ctx) override;

		antlrcpp::Any visitAggregate(SparqlParser::AggregateContext *ctx) override;

		antlrcpp::Any visitRdfLiteral(SparqlParser::RdfLiteralContext *) override;

		antlrcpp::Any visitNumericLiteral(SparqlParser::NumericLiteralContext *) override;

		antlrcpp::Any visitBooleanLiteral(SparqlParser::BooleanLiteralContext *) override;

		antlrcpp::Any visitString(SparqlParser::StringContext *) override;

	private:

		/**
		 * @brief: Creates dependencies between the operands of group graph patterns
		 */
		void group_dependencies(std::vector<uint8_t> const &prev_group, std::vector<uint8_t> const &cur_group, bool bidirectional = false);

		/**
		 * @brief: Visitor for well-designed SPARQL patterns
		 */
		void visitWellDesignedPattern(SparqlParser::GroupGraphPatternContext *ctx,
									  std::vector<SparqlParser::GroupOrUnionGraphPatternContext *> gou_ctxs);

	};

}// namespace dice::sparql2tensor::parser::visitors

#endif//DICE_SPARQL_SELECTASKQUERYVISITOR_HPP