#ifndef DICE_SPARQL_SELECTQUERYVISITOR_HPP
#define DICE_SPARQL_SELECTQUERYVISITOR_HPP

#include <rdf4cpp/rdf.hpp>

#include <SparqlParser/SparqlParserBaseVisitor.h>

#include "Dice/sparql2tensor/SPARQLQuery.hpp"

#include <robin_hood.h>

namespace Dice::sparql2tensor::parser::visitors {

	using namespace sparql_parser::base;

	class SelectAskQueryVisitor : public SparqlParserBaseVisitor {

	private:
		SPARQLQuery *const query;
		rdf4cpp::rdf::Node active_subject;
		rdf4cpp::rdf::Node active_predicate;
		char var_id = 'a';
		// for the construction of the operand dependency graph
		std::deque<std::vector<uint8_t>> group_patterns;
		std::deque<std::vector<uint8_t>> opt_operands;
	public:
		SelectAskQueryVisitor() = delete;

		explicit SelectAskQueryVisitor(SPARQLQuery *q) : query{q} {}

		antlrcpp::Any visitAskQuery(SparqlParser::AskQueryContext *ctx) override;

		antlrcpp::Any visitSelectQuery(SparqlParser::SelectQueryContext *) override;

		antlrcpp::Any visitSelectClause(SparqlParser::SelectClauseContext *) override;

		antlrcpp::Any visitWhereClause(SparqlParser::WhereClauseContext *) override;

		antlrcpp::Any visitGroupGraphPattern(SparqlParser::GroupGraphPatternContext *) override;

		antlrcpp::Any visitGroupGraphPatternSub(SparqlParser::GroupGraphPatternSubContext *) override;

		antlrcpp::Any visitTriplesBlock(SparqlParser::TriplesBlockContext *) override;

		antlrcpp::Any visitTriplesSameSubjectPath(SparqlParser::TriplesSameSubjectPathContext *) override;

		antlrcpp::Any visitPropertyListPathNotEmpty(SparqlParser::PropertyListPathNotEmptyContext *) override;

		antlrcpp::Any visitTriplesNodePath(SparqlParser::TriplesNodePathContext *) override;

		antlrcpp::Any visitBlankNodePropertyListPath(SparqlParser::BlankNodePropertyListPathContext *) override;

		antlrcpp::Any visitGraphPatternNotTriples(SparqlParser::GraphPatternNotTriplesContext *) override;

		antlrcpp::Any visitOptionalGraphPattern(SparqlParser::OptionalGraphPatternContext *) override;

		antlrcpp::Any visitGroupOrUnionGraphPattern(SparqlParser::GroupOrUnionGraphPatternContext *) override;

		antlrcpp::Any visitMinusGraphPattern(SparqlParser::MinusGraphPatternContext *) override;

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

		antlrcpp::Any visitRdfLiteral(SparqlParser::RdfLiteralContext *) override;

		antlrcpp::Any visitNumericLiteral(SparqlParser::NumericLiteralContext *) override;

		antlrcpp::Any visitBooleanLiteral(SparqlParser::BooleanLiteralContext *) override;

		antlrcpp::Any visitString(SparqlParser::StringContext *) override;

	private:
		void register_var(rdf4cpp::rdf::query::Variable const &);

		/**
		 * @brief: Creates a new node in the operand dependency graph.
		 * Creates dependencies for the new node within the same group graph pattern
		 */
		void add_tp(rdf4cpp::rdf::query::TriplePattern const &tp);

		/**
		 * @brief: Creates dependencies between group graph patterns
		 */
		void group_dependencies(std::vector<uint8_t> const &prev_group,
								std::vector<uint8_t> const &cur_group,
								bool bidirectional = false,
								bool is_union = false);

		/**
		 * @brief: Creates simple connections between group graph patterns (important to capture optional cartesian products)
		 */
		void group_connections(std::vector<uint8_t> const &prev_group, std::vector<uint8_t> const &cur_group);

		void visitWellDesignedPattern(SparqlParser::GroupGraphPatternSubContext *);

		void visitWellDesignedOptionalPattern(SparqlParser::OptionalGraphPatternContext *);

		void visitWellDesignedUnionPattern(SparqlParser::GroupOrUnionGraphPatternContext *);
	};

}// namespace Dice::sparql2tensor::parser::visitors

#endif//DICE_SPARQL_SELECTQUERYVISITOR_HPP