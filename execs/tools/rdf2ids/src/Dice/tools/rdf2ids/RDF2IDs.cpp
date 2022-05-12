#include <chrono>
#include <filesystem>

#include <csv.hpp>
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <tsl/sparse_set.h>

#include "tentris_version.hpp"

#include "SerdParser.hpp"

int main(int argc, char *argv[]) {
	using namespace Dice;
	namespace fs = std::filesystem;

	/*
	 * Parse Commandline Arguments
	 */
	std::string version = fmt::format("rdf2ids v{} is based on rdf4cpp {}.", Dice::tentris::version, Dice::tentris::rdf4cpp_version);

	cxxopts::Options options("rdf2ids",
							 fmt::format("{}\nConverting RDF 2 ID triples in tsv format. Result is written to stdout", version));
	options.add_options()                                                                                                                                        //
			("d,distinct", "Store each id triple only once.", cxxopts::value<bool>()->default_value("false"))                                                    //
			("f,file", "TURTLE or NTRIPLE RDF file that should be processed.", cxxopts::value<std::string>())                                                    //
			("m,limit", "Maximum number of id triples returned.", cxxopts::value<size_t>()->default_value(fmt::format("{}", std::numeric_limits<size_t>::max())))//
			("l,loglevel", fmt::format("Details of logging. Available values are: [{}, {}, {}, {}, {}, {}, {}]",                                                 //
									   spdlog::level::to_string_view(spdlog::level::trace),                                                                      //
									   spdlog::level::to_string_view(spdlog::level::debug),                                                                      //
									   spdlog::level::to_string_view(spdlog::level::info),                                                                       //
									   spdlog::level::to_string_view(spdlog::level::warn),                                                                       //
									   spdlog::level::to_string_view(spdlog::level::err),                                                                        //
									   spdlog::level::to_string_view(spdlog::level::critical),                                                                   //
									   spdlog::level::to_string_view(spdlog::level::off)),                                                                       //
			 cxxopts::value<std::string>()->default_value("info"))                                                                                               //
			("v,version", "Version info.")                                                                                                                       //
			("h,help", "Print this help page.")                                                                                                                  //
			;

	auto parsed_args = options.parse(argc, argv);
	if (parsed_args.count("help")) {
		std::cerr << options.help() << std::endl;
		exit(0);
	} else if (parsed_args.count("version")) {
		std::cerr << version << std::endl;
		exit(0);
	}

	/*
	 * Initialize logger
	 */
	const auto log_level = spdlog::level::from_str(parsed_args["loglevel"].as<std::string>());
	spdlog::set_default_logger(spdlog::stderr_color_mt("rdf2ids logger"));
	spdlog::set_level(log_level);
	spdlog::set_pattern("%Y-%m-%dT%T.%e%z | %n | %t | %l | %v");
	spdlog::info(version);

	auto const limit = parsed_args["limit"].as<size_t>();
	size_t count = 0;

	// write TSV to std::cout
	auto tsv_writer = csv::make_tsv_writer(std::cout);
	{
		// we want to write ID triples
		using IDTriple = std::tuple<uint64_t, uint64_t, uint64_t>;
		static Dice::hash::DiceHashxxh3<IDTriple> hasher{};

		// terminate when the limit is reached
		auto terminate_at_limit = [&count, &limit, &tsv_writer] {
			if (++count > limit) {
				tsv_writer.flush();
				spdlog::info("Limit of {} entries reached.", limit);
				spdlog::info("Shutdown successful.");
				return exit(0);
			}
		};

		// callback that produces unique id triples
		auto distinct_callback = [&](rdf4cpp::rdf::Node s, rdf4cpp::rdf::Node p, rdf4cpp::rdf::Node o) {
			static tsl::sparse_set<uint64_t> deduplication;

			IDTriple id_triple = std::make_tuple(s.backend_handle().raw(), p.backend_handle().raw(), o.backend_handle().raw());
			auto hash = hasher(id_triple);
			if (not deduplication.contains(hash)) {
				terminate_at_limit();
				tsv_writer << id_triple;
				deduplication.insert(hash);
			}
		};

		// callback that allows duplicates
		auto bag_callback = [&](rdf4cpp::rdf::Node s, rdf4cpp::rdf::Node p, rdf4cpp::rdf::Node o) {
			terminate_at_limit();

			IDTriple id_triple = std::make_tuple(s.backend_handle().raw(), p.backend_handle().raw(), o.backend_handle().raw());
			tsv_writer << id_triple;
		};

		using namespace Dice::tools::rdf2ids::serd_parser;

		// start serd parser
		SerdHandle serd_handle{
				.prefixes = {},
				.add_triple_callback = (parsed_args["distinct"].as<bool>()) ? TripleParsed_callback{distinct_callback} : TripleParsed_callback{bag_callback}};

		SerdReader *reader = serd_reader_new(SERD_TURTLE, (void *) &serd_handle,
											 nullptr,
											 reinterpret_cast<SerdBaseSink>(on_base),
											 reinterpret_cast<SerdPrefixSink>(on_prefix),
											 reinterpret_cast<SerdStatementSink>(on_statement),
											 reinterpret_cast<SerdEndSink>(on_end));
		serd_reader_read_file(reader, reinterpret_cast<const uint8_t *>(parsed_args["file"].as<std::string>().c_str()));
		serd_reader_free(reader);
	}

	spdlog::info("Shutdown successful.");
	return EXIT_SUCCESS;
}
