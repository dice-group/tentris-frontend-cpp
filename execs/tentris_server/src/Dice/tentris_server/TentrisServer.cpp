#include <chrono>
#include <filesystem>

#include <cxxopts.hpp>
#include <fmt/format.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <taskflow/taskflow.hpp>

#include <Dice/endpoint/HTTPServer.hpp>
#include <Dice/node_store/PersistentNodeStorageBackend.hpp>
#include <Dice/triple_store/TripleStore.hpp>

#include "tentris_version.hpp"

int main(int argc, char *argv[]) {
	using namespace Dice;
	namespace fs = std::filesystem;

	/*
	 * Parse Commandline Arguments
	 */
	std::string version = fmt::format("tentris_server v{} is based on hypertrie v{} and rdf4cpp v{}.", Dice::tentris::version, hypertrie::version, "pre-release");

	cxxopts::Options options("tentris_server",
							 fmt::format("{}\nA tensor-based triple store.", version));
	options.add_options()                                                                                                                                                                                                                                    //
			("s,storage", "Location where the index is stored.", cxxopts::value<std::string>()->default_value(fs::current_path().string()))("f,file", "A N-Triples or Turtle file. Will be loaded before the endpoint starts", cxxopts::value<std::string>())//
			("b,bulksize", "Bulk-size for loading RDF files. A larger value results in a higher memory consumption during loading RDF data but may result in shorter loading times.", cxxopts::value<size_t>()->default_value("1000000"))                    //
			("t,timeout", "Time out in seconds for answering requests.", cxxopts::value<uint>()->default_value("180"))                                                                                                                                       //
			("j,threads", "Number of threads used by the endpoint.", cxxopts::value<size_t>()->default_value(std::to_string(std::thread::hardware_concurrency())))                                                                                           //
			("p,port", "Port to be used by the endpoint.", cxxopts::value<uint16_t>()->default_value("9080"))                                                                                                                                                //
			("l,loglevel", fmt::format("Details of logging. Available values are: [{}, {}, {}, {}, {}, {}, {}]",                                                                                                                                             //
									   spdlog::level::to_string_view(spdlog::level::trace),                                                                                                                                                                  //
									   spdlog::level::to_string_view(spdlog::level::debug),                                                                                                                                                                  //
									   spdlog::level::to_string_view(spdlog::level::info),                                                                                                                                                                   //
									   spdlog::level::to_string_view(spdlog::level::warn),                                                                                                                                                                   //
									   spdlog::level::to_string_view(spdlog::level::err),                                                                                                                                                                    //
									   spdlog::level::to_string_view(spdlog::level::critical),                                                                                                                                                               //
									   spdlog::level::to_string_view(spdlog::level::off)),                                                                                                                                                                   //
			 cxxopts::value<std::string>()->default_value("info"))                                                                                                                                                                                           //
			("logfile", "If log is written to files.", cxxopts::value<bool>()->default_value("true"))                                                                                                                                                        //
			("logstdout", "If log is written to stdout.", cxxopts::value<bool>()->default_value("false"))                                                                                                                                                    //
			("logfiledir", "A folder path where to write the logfiles. Default is the current working directory.", cxxopts::value<std::string>()->default_value(fs::current_path().string()))                                                                //
			("v,version", "Version info.")                                                                                                                                                                                                                   //
			("h,help", "Print this help page.")                                                                                                                                                                                                              //
			;

	auto parsed_args = options.parse(argc, argv);
	if (parsed_args.count("help")) {
		std::cout << options.help() << std::endl;
		exit(0);
	} else if (parsed_args.count("version")) {
		std::cout << version << std::endl;
		exit(0);
	}

	/*
	 * Initialize logger
	 */
	const auto log_level = spdlog::level::from_str(parsed_args["loglevel"].as<std::string>());
	spdlog::set_level(log_level);

	std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks;

	if (parsed_args["logfile"].as<bool>()) {
		// Create a file rotating logger with 5mb size max and 10 rotated files
		const auto max_size = 1048576 * 5;
		const auto max_files = 10;
		auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(parsed_args["logfiledir"].as<std::string>() + "/tentris.log", max_size, max_files);
		file_sink->set_level(log_level);
		sinks.emplace_back(std::move(file_sink));
	}

	if (parsed_args["logstdout"].as<bool>()) {
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(log_level);
		sinks.emplace_back(std::move(console_sink));
	}

	auto logger = std::make_shared<spdlog::logger>("tentris logger", sinks.begin(), sinks.end());
	logger->set_level(log_level);
	spdlog::set_default_logger(logger);
	spdlog::set_pattern("%Y-%m-%dT%T.%e%z | %n | %t | %l | %v");
	spdlog::info(version);
	spdlog::flush_every(std::chrono::seconds{5});


	/*
	 * Initialize endpoint, executors and storage
	 */
	const endpoint::EndpointCfg endpoint_cfg{
			.port = parsed_args["port"].as<uint16_t>(),
			.threads = parsed_args["port"].as<uint16_t>(),
			.timeout_duration = std::chrono::seconds{parsed_args["timeout"].as<uint>()}};

	auto const storage_path = fs::absolute(fs::path{parsed_args["storage"].as<std::string>()}).append("tentris_data");
	if (not metall::manager ::consistent(storage_path.c_str())) {
		spdlog::info("No index storage or corrupted index storage found at {}. New storage is initialized.", storage_path);
		metall::manager{metall::create_only, storage_path.c_str()};
	} else {
		spdlog::info("Existing index storage at {}.", storage_path);
	}
	metall::manager storage_manager{metall::open_only, storage_path.c_str()};

	// setting up node storage
	namespace node_storage_n = rdf4cpp::rdf::storage::node;
	using NodeStorage = node_storage_n::NodeStorage;
	auto *backend_impl = storage_manager.find_or_construct<Dice::node_store::PersistentNodeStorageBackendImpl>("node_store")(storage_manager.get_allocator());
	Dice::node_store::PersistentNodeStorageBackend backend{backend_impl};
	auto nodestorage = NodeStorage::register_backend(&backend);
	NodeStorage::primary_instance(nodestorage);
	auto std_storage = NodeStorage::new_instance();// necessary for initialization

	triple_store::TripleStore &triplestore = *storage_manager.find_or_construct<triple_store::TripleStore>("triple_store")(storage_manager.get_allocator());
	tf::Executor executor(endpoint_cfg.threads);

	endpoint::HTTPServer endpoint{executor, triplestore, endpoint_cfg};

	/*
	 * Load data into triplestore
	 */
	if (parsed_args.count("file")) {
		fs::path ttl_file(parsed_args["file"].as<std::string>());

		spdlog::info("Loading triples from file {}.", fs::absolute(ttl_file));
		spdlog::stopwatch loading_time;
		spdlog::stopwatch batch_loading_time;
		size_t total_processed_entries = 0;
		size_t total_inserted_entries = 0;
		size_t final_hypertrie_size_after = 0;
		triplestore.load_ttl(parsed_args["file"].as<std::string>(),
							 parsed_args["bulksize"].as<size_t>(),
							 [&](size_t processed_entries,
								 size_t inserted_entries,
								 size_t hypertrie_size_after) -> void {
								 std::chrono::duration<double> batch_duration = batch_loading_time.elapsed();
								 spdlog::info("  batch: {:>10.3} mio triples processed, {:>10.3} mio triples added, {} elapsed, {:>10.3} mio triples in storage.",
											  (double(processed_entries) / 1'000'000),
											  (double(inserted_entries) / 1'000'000),
											  batch_duration,
											  (double(hypertrie_size_after) / 1'000'000));
								 total_processed_entries = processed_entries;
								 total_inserted_entries = inserted_entries;
								 final_hypertrie_size_after = hypertrie_size_after;
								 batch_loading_time.reset();
							 });
		spdlog::info("  loading finished: {} triples processed, {} triples added, {} elapsed, {} triples in storage.",
					 total_processed_entries, total_inserted_entries, loading_time.elapsed(), final_hypertrie_size_after);
	}
	const auto cards = triplestore.get_hypertrie().get_cards({0, 1, 2});
	spdlog::info("Storage stats: {} triples ({} distinct subjects, {} distinct predicates, {} distinct objects)",
				 triplestore.size(), cards[0], cards[1], cards[2]);
	spdlog::info("SPARQL endpoint serving sparkling linked data treasures on {} threads at http://0.0.0.0:{}/ with {} request timeout.",
				 endpoint_cfg.threads, endpoint_cfg.port, endpoint_cfg.timeout_duration);
	endpoint();

	// warping up node storage
	NodeStorage::unregister_backend(&backend);
	NodeStorage::primary_instance(std_storage);
	spdlog::info("Shutdown successful.");
	return EXIT_SUCCESS;
}
