#include <chrono>
#include <filesystem>
#include <iostream>

#include <tentris/store/RDF/SerdParser2.hpp>
#include <tentris/store/RDF/TermStore.hpp>
#include <tentris/store/TripleStore.hpp>

int main(int argc, char *argv[]) {
	using namespace tentris::store;
	using namespace fmt::literals;
	using namespace std::chrono;

	if (argc != 2) {
		std::cerr << "Please provide exactly one triple file as commandline argument." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::string rdf_file{argv[1]};
	if (not std::filesystem::is_regular_file(rdf_file)) {
		std::cerr << "{} is not a file."_format(rdf_file) << std::endl;
		exit(EXIT_FAILURE);
	}

	fmt::print(stderr, "To store the result pipe stdout (not stderr!) to a file. Output format is TSV (tab-separated file, extension: .tsv).\n");

	rdf::TermStore ts{};
	unsigned long count = 0;

	TripleStore triple_store{};

	try {
		auto start = steady_clock::now();
		fmt::print("S\tP\tO\n");
		triple_store.bulkloadRDF(rdf_file);
		for (auto const &entry : triple_store.getBoolHypertrie()) {
			fmt::print("{}\t{}\t{}\n", intptr_t(entry[0]), intptr_t(entry[1]), intptr_t(entry[2]));
		}
		auto end = steady_clock::now();
		auto duration = end - start;

		fmt::print(stderr, "total triples processed: {}\n", count);
		fmt::print(stderr, "duration: {} h {} min {}.{:03d} s = {} ms\n",
				   (std::chrono::duration_cast<std::chrono::hours>(duration)).count(),
				   (std::chrono::duration_cast<std::chrono::minutes>(duration) % 60).count(),
				   (std::chrono::duration_cast<std::chrono::seconds>(duration) % 60).count(),
				   (std::chrono::duration_cast<std::chrono::milliseconds>(duration) % 1000).count(),
				   std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
	} catch (...) {
		throw std::invalid_argument{
				"A parsing error occurred while parsing {}. Error occurred at {}th triple."_format(rdf_file, count)};
	}
}
