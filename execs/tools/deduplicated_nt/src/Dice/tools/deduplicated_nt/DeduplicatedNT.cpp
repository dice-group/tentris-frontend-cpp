#include <chrono>
#include <filesystem>

#include <Dice/hash/DiceHash.hpp>
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
	std::string version = fmt::format("deduplicate_nt v{} is based on rdf4cpp {}.", Dice::tentris::version, Dice::tentris::rdf4cpp_version);

	cxxopts::Options options("deduplicate_nt",
							 fmt::format("{}\nDeduplicating RDF files (TURTLE, NTRIPLE). Result is serialized in NTRIPLE on console out. Logs are written to console error.", version));
	options.add_options()                                                                                                                                                                              //
			("f,file", "TURTLE or NTRIPLE RDF file that should be processed.", cxxopts::value<std::string>())                                                                                          //
			("m,limit", "Maximum number of result triples. When the limit is reached, the tool quits.", cxxopts::value<size_t>()->default_value(fmt::format("{}", std::numeric_limits<size_t>::max())))//
			("l,loglevel", fmt::format("Details of logging. Available values are: [{}, {}, {}, {}, {}, {}, {}]",                                                                                       //
									   spdlog::level::to_string_view(spdlog::level::trace),                                                                                                            //
									   spdlog::level::to_string_view(spdlog::level::debug),                                                                                                            //
									   spdlog::level::to_string_view(spdlog::level::info),                                                                                                             //
									   spdlog::level::to_string_view(spdlog::level::warn),                                                                                                             //
									   spdlog::level::to_string_view(spdlog::level::err),                                                                                                              //
									   spdlog::level::to_string_view(spdlog::level::critical),                                                                                                         //
									   spdlog::level::to_string_view(spdlog::level::off)),                                                                                                             //
			 cxxopts::value<std::string>()->default_value("info"))                                                                                                                                     //
			("v,version", "Version info.")                                                                                                                                                             //
			("h,help", "Print this help page.")                                                                                                                                                        //
			;

	auto parsed_args = options.parse(argc, argv);
	if (parsed_args.count("help")) {
		std::cerr << options.help() << std::endl;
		exit(EXIT_SUCCESS);
	} else if (parsed_args.count("version")) {
		std::cerr << version << std::endl;
		exit(EXIT_SUCCESS);
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
	{
		// we want to write ID triples
		using IDTriple = std::tuple<uint64_t, uint64_t, uint64_t>;
		static Dice::hash::DiceHashxxh3<IDTriple> hasher{};

		// terminate when the limit is reached
		auto terminate_at_limit = [&count, &limit] {
			if (++count > limit) {
				std::cout.flush();
				spdlog::info("Limit of {} triples reached.", limit);
				spdlog::info("Shutdown successful.");
				exit(EXIT_SUCCESS);
			}
		};

		// callback that produces unique id triples
		auto distinct_callback = [&](rdf4cpp::rdf::Node s, rdf4cpp::rdf::Node p, rdf4cpp::rdf::Node o) {
			static tsl::sparse_set<uint64_t> deduplication;

			IDTriple id_triple = std::make_tuple(s.backend_handle().raw(), p.backend_handle().raw(), o.backend_handle().raw());
			auto hash = hasher(id_triple);
			if (not deduplication.contains(hash)) {
				terminate_at_limit();
				std::cout << fmt::format("{} {} {} . \n", std::string(s), std::string(p), std::string(o));
				deduplication.insert(hash);
			}
		};

		using namespace Dice::tools::rdf2ids::serd_parser;

		// start serd parser
		SerdHandle serd_handle{
				.prefixes = {},
				.add_triple_callback = distinct_callback};

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
