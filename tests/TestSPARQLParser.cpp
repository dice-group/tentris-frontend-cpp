#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <regex>

#include <Dice/sparql/parser/ParsedSPARQL.hpp>
#include <Dice/sparql/parser/Parser.hpp>
#include <rdf4cpp/rdf.hpp>
#include <tentris/util/FmtHelper.hpp>

namespace tentris::tests::sparql_parser {
	using namespace Dice::sparql::parser;
	namespace fs = std::filesystem;
	using Term = rdf4cpp::rdf::Node;
	using BNode = rdf4cpp::rdf::BlankNode;
	using Literal = rdf4cpp::rdf::Literal;
	using URIRef = rdf4cpp::rdf::IRI;
	using Triple = rdf4cpp::rdf::Statement;
	using TriplePattern = rdf4cpp::rdf::query::TriplePattern;
	using Variable = rdf4cpp::rdf::query::Variable;

	std::vector<std::string> load_queries(const std::string &query_file_path) {
		std::vector<std::string> queries{};
		std::ifstream query_file(query_file_path);
		std::string query;
		while (std::getline(query_file, query)) {
			static std::regex white_spaces{"\\s+"};
			if (not std::regex_match(query, white_spaces))
				queries.push_back(query);
		}
		return queries;
	}

	bool parse_queries(const std::string &file) {
		std::vector<std::string> queries = load_queries(file);

		bool no_errors = true;
		fmt::print(" ### {}\n", file);
		std::set<std::size_t> lines_with_errors{};
		for (std::size_t i = 0; i < queries.size(); i++) {
			fmt::print(" # {:3d} {}\n", i + 1, queries[i]);
			try {
				ParsedSPARQL query = parse_query(queries[i]);
				fmt::print(" #     OK\n");
			} catch (std::exception &ex) {
				lines_with_errors.insert(i + 1);
				no_errors = false;
				fmt::print(" #     failed\n");
				fmt::print("{}\n", ex.what());
				std::cout << ex.what() << std::endl;
			}
		}
		fmt::print(" ### failed queries: {}\n", fmt::join(lines_with_errors, ","));
		return no_errors;
	}

	TEST(TestSPARQLParser, ParseBenchmarkQueries) {
		namespace fs = std::filesystem;
		const std::string sp2b_file = "/home/nkaralis/CLionProjects/tentris-private/tests/queries/sp2b.txt";
		const std::string dbpedia_file = "/home/nkaralis/CLionProjects/tentris-private/tests/queries/DBpedia.txt";
		const std::string swdf_file = "/home/nkaralis/CLionProjects/tentris-private/tests/queries/swdf.txt";
		ASSERT_TRUE(fs::is_regular_file(sp2b_file));
		ASSERT_TRUE(fs::is_regular_file(dbpedia_file));
		ASSERT_TRUE(fs::is_regular_file(swdf_file));

		bool no_errors = true;
		no_errors &= parse_queries(sp2b_file);
		no_errors &= parse_queries(dbpedia_file);
		no_errors &= parse_queries(swdf_file);
		ASSERT_TRUE(no_errors);
	}

	TEST(TestSPARQLParser, ParseSingleQuery) {

		const std::string query = "PREFIX  dc:   <http://purl.org/dc/elements/1.1/>  PREFIX  :     <http://dbpedia.org/resource/>  PREFIX  rdfs: <http://www.w3.org/2000/01/rdf-schema#>  PREFIX  dbpedia2: <http://dbpedia.org/property/>  PREFIX  foaf: <http://xmlns.com/foaf/0.1/>  PREFIX  owl:  <http://www.w3.org/2002/07/owl#>  PREFIX  xsd:  <http://www.w3.org/2001/XMLSchema#>  PREFIX  dbpedia: <http://dbpedia.org/>  PREFIX  rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#>  PREFIX  skos: <http://www.w3.org/2004/02/skos/core#>  "
								  " SELECT  *  WHERE    { "
								  "?data1 rdf:type <http://dbpedia.org/ontology/FormulaOneRacer> .    "
								  "  ?wins2 <http://dbpedia.org/ontology/wins> 10 .  "
								  "?data3 rdf:type <http://dbpedia.org/ontology/FormulaOneRacer> .    "
								  " }";
		ParsedSPARQL q = parse_query(query);
	}

	TEST(TestSPARQLParser, ParseSingleQuery2) {

		const std::string query = ""
								  " SELECT  *  WHERE    { _:data <http://example.com/pred> ?b }";

		ParsedSPARQL q = parse_query(query);
	}

	TEST(TestSPARQLParser, parse_a_query) {

		const std::string query = "PREFIX rdf:     <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX bench:   <http://localhost/vocabulary/bench/> PREFIX dc:      <http://purl.org/dc/elements/1.1/> PREFIX dcterms: <http://purl.org/dc/terms/> PREFIX foaf:    <http://xmlns.com/foaf/0.1/> PREFIX swrc:    <http://swrc.ontoware.org/ontology#>  "
								  "SELECT DISTINCT ?name1 ?name2  WHERE {   "
								  "?article1 rdf:type bench:Article .   "
								  "?article2 rdf:type bench:Article .   "
								  "?article1 dc:creator ?author1 .   "
								  "?author1 foaf:name ?name1 .   "
								  "?article2 dc:creator ?author2 .   "
								  "?author2 foaf:name ?name2 .   "
								  "?author2 foaf:name \"nikos\"@en .   "
								  "?article1 swrc:journal ?journal .   "
								  "?article2 swrc:journal ?journal }";
		ParsedSPARQL q = parse_query(query);

		const Term type = rdf4cpp::rdf::IRI("http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
		const Term creator = rdf4cpp::rdf::IRI("http://purl.org/dc/elements/1.1/creator");
		const Term journal = rdf4cpp::rdf::IRI("http://swrc.ontoware.org/ontology#journal");
		const Term name = rdf4cpp::rdf::IRI("http://xmlns.com/foaf/0.1/name");

		const Term article = rdf4cpp::rdf::IRI("http://localhost/vocabulary/bench/Article");

		const Variable article1 = Variable{"article1"};
		const Variable article2 = Variable{"article2"};
		const Variable author1 = Variable{"author1"};
		const Variable author2 = Variable{"author2"};
		const Variable name1 = Variable{"name1"};
		const Variable name2 = Variable{"name2"};
		const Variable journal_var = Variable{"journal"};
		const Literal name_lang = Literal{"nikos", "@en"};
		std::vector<TriplePattern> actual_bgps{
				{article1, type, article},
				{article2, type, article},
				{article1, creator, author1},
				{author1, name, name1},
				{article2, creator, author2},
				{author2, name, name2},
				{author2, name, name_lang},
				{article1, journal, journal_var},
				{article2, journal, journal_var}};

		auto query_variables = std::vector{name1, name2};

		ASSERT_EQ(q.prefixes.size(), 6);

		ASSERT_EQ(q.triple_patterns.size(), 9);

		ASSERT_EQ(q.var_to_id.size(), 7);

		ASSERT_EQ(q.projected_variables, query_variables);

		ASSERT_EQ(q.triple_patterns, actual_bgps);

		ASSERT_EQ(q.distinct, true);

		ASSERT_EQ(q.odg.size(), q.triple_patterns.size());

		ASSERT_EQ(q.odg.weaklyConnectedComponents().size(), 1);

		ASSERT_EQ(q.odg.cartesianComponents().size(), 1);
	}
}// namespace tentris::tests::sparql_parser
