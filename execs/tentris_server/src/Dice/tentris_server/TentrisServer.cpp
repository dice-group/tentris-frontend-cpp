

#include <Dice/triple_store/TripleStore.hpp>

#include <restinio/all.hpp>

#include <fmt/format.h>

//#include "VersionStrings.hpp"

#include <chrono>
#include <csignal>
#include <cxxopts.hpp>
#include <filesystem>
#include <spdlog/logger.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <taskflow/taskflow.hpp>

#include "Dice/endpoint/Endpoint.hpp"
#include "Dice/endpoint/SparqlJsonResultSAXWriter.hpp"
#include <memory>

int main(int argc, char *argv[]) {
	using namespace Dice;
	namespace fs = std::filesystem;

	std::string version = fmt::format("tentris_server v{} is based on hypertrie v{} and rdf4cpp v{}.", 1, hypertrie::version, "pre-release");

	cxxopts::Options options("tentris_server",
							 fmt::format("{}\nA tensor-based triple store.", version));
	options.add_options()                                                                                                                                                                                                                //
			("f,file", "A N-Triples or Turtle file. Will be loaded before the endpoint starts", cxxopts::value<std::string>())                                                                                                           //
			("b,bulksize", "Bulk-size for loading RDF files. A larger value results in a higher memory consumption during loading RDF data but may result in shorter loading times.", cxxopts::value<size_t>()->default_value("1000000"))//
			("t,timeout", "Time out in seconds for answering requests.", cxxopts::value<uint>()->default_value("180"))                                                                                                                   //
			("j,threads", "Number of threads used by the endpoint.", cxxopts::value<size_t>()->default_value(std::to_string(std::thread::hardware_concurrency())))                                                                       //
			("p,port", "Port to be used by the endpoint.", cxxopts::value<uint16_t>()->default_value("9080"))                                                                                                                            //
			("l,loglevel", fmt::format("Details of logging. Available values are: [{}, {}, {}, {}, {}, {}, {}]",
									   spdlog::level::to_string_view(spdlog::level::trace),                                                                                                  //
									   spdlog::level::to_string_view(spdlog::level::debug),                                                                                                  //
									   spdlog::level::to_string_view(spdlog::level::info),                                                                                                   //
									   spdlog::level::to_string_view(spdlog::level::warn),                                                                                                   //
									   spdlog::level::to_string_view(spdlog::level::err),                                                                                                    //
									   spdlog::level::to_string_view(spdlog::level::critical),                                                                                               //
									   spdlog::level::to_string_view(spdlog::level::off)),                                                                                                   //
			 cxxopts::value<std::string>()->default_value("info"))                                                                                                                           //
			("logfile", "If log is written to files.", cxxopts::value<bool>()->default_value("true"))                                                                                        //
			("logstdout", "If log is written to stdout.", cxxopts::value<bool>()->default_value("false"))                                                                                    //
			("logfiledir", "A folder path where to write the logfiles. Default is the current working directory.", cxxopts::value<std::string>()->default_value(fs::current_path().string()))//
			("v,version", "Version info.")                                                                                                                                                   //
			("h,help", "Print this help page.")                                                                                                                                              //
			;

	auto parsed_args = options.parse(argc, argv);
	if (parsed_args.count("help")) {
		std::cout << options.help() << std::endl;
		exit(0);
	} else if (parsed_args.count("version")) {
		std::cout << version << std::endl;
		exit(0);
	}

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
	spdlog::info("Starting tentris_server");
	spdlog::info(version);
	spdlog::flush_every(std::chrono::seconds{5});

	triple_store::TripleStore triplestore{};

	// TODO: load after everything is initialized
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
								 spdlog::info("batch: {:>10.3} mio triples processed, {:>10.3} mio triples added, {} elapsed | {:>10.3} mio triples in storage.",
											  (double(processed_entries) / 1'000'000),
											  (double(inserted_entries) / 1'000'000),
											  batch_duration,
											  (double(hypertrie_size_after) / 1'000'000));
								 total_processed_entries = processed_entries;
								 total_inserted_entries = inserted_entries;
								 final_hypertrie_size_after = hypertrie_size_after;
								 batch_loading_time.reset();
							 });
		spdlog::info("loading finished: {} triples processed, {} triples added, {} elapsed | {} triples in storage.",
					 total_processed_entries, total_inserted_entries, loading_time.elapsed(), final_hypertrie_size_after);
	}

	const endpoint::EndpointCfg endpoint_cfg{
			.port = parsed_args["port"].as<uint16_t>(),
			.threads = parsed_args["port"].as<uint16_t>()};
	std::chrono::seconds timeout_duration{parsed_args["timeout"].as<uint>()};

	tf::Executor executor(endpoint_cfg.threads);

	endpoint::Endpoint endpoint{executor, endpoint_cfg};

	endpoint.router()
			.http_get(R"(/sparql)",
					  [&](restinio::request_handle_t req, const auto &) -> restinio::request_handling_status_t {
						  if (executor.num_topologies() < endpoint_cfg.threads) {
							  executor.silent_async([&triplestore, timeout_duration](restinio::request_handle_t req) {
								  using namespace Dice::sparql2tensor;

								  const auto qp = restinio::parse_query(req->header().query());
								  if (not qp.has("query"))
									  return req->create_response(restinio::status_bad_request()).set_body("Query parameter 'query' is missing.").done();
								  std::string sparql_query_str = std::string{qp["query"]};
								  SPARQLQuery sparql_query;
								  try {
									  sparql_query = SPARQLQuery::parse(sparql_query_str);
								  } catch (std::exception &ex) {
									  return req->create_response(restinio::status_bad_request()).set_body("Failed to parse query.").done();
								  }


								  endpoint::SparqlJsonResultSAXWriter json_writer{sparql_query.projected_variables_, 100'000};

								  for (auto const &entry : triplestore.query(sparql_query, std::chrono::steady_clock::now() + timeout_duration)) {
									  json_writer.add(entry);
								  }
								  return req->create_response(restinio::status_ok()).set_body(json_writer.string_view()).done();
							  },
													std::move(req));
							  return restinio::request_accepted();
						  } else {
							  return restinio::request_rejected();
						  }
					  });

	endpoint();

	//
	//	// create endpoint
	//	using namespace restinio;
	//	auto router = std::make_unique<router::express_router_t<>>();
	//	router->http_get(
	//			R"(/sparql2tensor)",
	//			tentris::http::sparql_endpoint::SparqlEndpoint<restinio::restinio_controlled_output_t>{});
	//	router->http_get(
	//			R"(/stream)",
	//			tentris::http::sparql_endpoint::SparqlEndpoint<restinio::chunked_output_t>{});
	//	router->http_get(
	//			R"(/count)",
	//			[&](restinio::request_handle_t req, auto const &) -> restinio::request_handling_status_t {
	//				using namespace ::std::chrono;
	//				using namespace ::tentris::logging;
	//				using namespace ::tentris::tensor;
	//				using AtomicQueryExecutionCache = ::tentris::store::AtomicQueryExecutionCache;
	//				using AtomicTripleStoreConfig = ::tentris::store::config::AtomicTripleStoreConfig;
	//				using QueryExecutionPackage = ::tentris::store::cache::QueryExecutionPackage;
	//				using SelectModifier = Dice::sparql::Nodes::QueryNodes::SelectNodes::SelectModifier;
	//
	//				auto start_time = steady_clock::now();
	//				log("count request started.");
	//				auto start_memory = get_memory_usage();
	//				logDebug("ram: {:d} kB"_format(start_memory));
	//				auto timeout = start_time + AtomicTripleStoreConfig::getInstance().timeout;
	//				std::shared_ptr<QueryExecutionPackage> query_package;
	//				std::string query_string{};
	//				try {
	//					const auto query_params = restinio::parse_query<restinio::parse_query_traits::javascript_compatible>(
	//							req->header().query());
	//					query_string = std::string(query_params["query"]);
	//					log("query: {}"_format(query_string));
	//					// check if there is actually an query
	//					try {
	//						query_package = AtomicQueryExecutionCache::getInstance()[query_string];
	//					} catch (const std::invalid_argument &exc) {
	//						logDebug(exc.what());
	//						return req->create_response(status_bad_request()).set_body("Query could not be parsed.").done();
	//					}
	//					size_t count = 0;
	//					try {
	//
	//						if (query_package->getSelectModifier() == SelectModifier::DISTINCT) {
	//							for ([[maybe_unused]] auto const &entry : Dice::einsum::einsum<DISTINCT_t, tr>(query_package->getSubscript(), query_package->getOperands(), timeout))
	//								++count;
	//						} else {
	//							for (auto const &entry : Dice::einsum::einsum<DISTINCT_t, tr>(query_package->getSubscript(), query_package->getOperands(), timeout))
	//								count += entry.value();
	//						}
	//
	//						log("result_count: {}"_format(count));
	//						return req->create_response().set_body(fmt::format("{}", count)).done();
	//
	//					} catch (Dice::einsum::TimeoutException const &ex) {
	//						return req->create_response(status_request_time_out())
	//								.set_body(fmt::format("Timed out after {} s and counting {} results.",
	//													  duration_cast<seconds>(AtomicTripleStoreConfig::getInstance().timeout).count()))
	//								.done();
	//					}
	//				} catch (const std::exception &exc) {
	//					// if the execution of the query should fail return an internal server error
	//					logDebug(exc.what());
	//					return req->create_response(status_bad_request()).set_body("?query parameter missing or faulty.").done();
	//				}
	//			});
	//
	//	router->http_get(
	//			R"(/ask)",
	//			[&](restinio::request_handle_t req, auto const &) -> restinio::request_handling_status_t {
	//				using namespace ::std::chrono;
	//				using namespace ::tentris::logging;
	//				using namespace ::tentris::tensor;
	//				using AtomicQueryExecutionCache = ::tentris::store::AtomicQueryExecutionCache;
	//				using AtomicTripleStoreConfig = ::tentris::store::config::AtomicTripleStoreConfig;
	//				using QueryExecutionPackage = ::tentris::store::cache::QueryExecutionPackage;
	//
	//				auto start_time = steady_clock::now();
	//				log("ask request started.");
	//				auto start_memory = get_memory_usage();
	//				logDebug("ram: {:d} kB"_format(start_memory));
	//				auto timeout = start_time + AtomicTripleStoreConfig::getInstance().timeout;
	//				std::shared_ptr<QueryExecutionPackage> query_package;
	//				std::string query_string{};
	//				try {
	//					const auto query_params = restinio::parse_query<restinio::parse_query_traits::javascript_compatible>(
	//							req->header().query());
	//					query_string = std::string(query_params["query"]);
	//					log("query: {}"_format(query_string));
	//					// check if there is actually an query
	//					try {
	//						query_package = AtomicQueryExecutionCache::getInstance()[query_string];
	//					} catch (const std::invalid_argument &exc) {
	//						logDebug(exc.what());
	//						return req->create_response(status_bad_request()).set_body("Query could not be parsed.").done();
	//					}
	//					try {
	//						auto ask_subscript = std::make_shared<Dice::einsum::Subscript>(query_package->getSubscript()->getRawSubscript().operands, Subscript::ResultSc{});
	//						bool ask = false;
	//						for (auto const &entry : Dice::einsum::einsum<DISTINCT_t, tr>(query_package->getSubscript(), query_package->getOperands(), timeout)) {
	//							if (entry.value()) {
	//								ask = true;
	//								break;
	//							}
	//						}
	//						log("ask: {}"_format(ask));
	//						return req->create_response().set_body(fmt::format("{}", ask)).done();
	//
	//					} catch (Dice::einsum::TimeoutException const &ex) {
	//						return req->create_response(status_request_time_out())
	//								.set_body(fmt::format("Timed out after {} s.",
	//													  duration_cast<seconds>(AtomicTripleStoreConfig::getInstance().timeout).count()))
	//								.done();
	//					}
	//				} catch (const std::exception &exc) {
	//					// if the execution of the query should fail return an internal server error
	//					logDebug(exc.what());
	//					return req->create_response(status_bad_request()).set_body("?query parameter missing or faulty.").done();
	//				}
	//			});
	//
	//	router->non_matched_request_handler(
	//			[](auto req) -> restinio::request_handling_status_t {
	//				return req->create_response(restinio::status_not_found()).connection_close().done();
	//			});
	//
	//	// Launching a server with custom traits.
	//
	//	log("SPARQL endpoint serving sparkling linked data treasures on {} threads at http://0.0.0.0:{}/sparql2tensor?query="_format(cfg.threads, cfg.port));
	//
	//	restinio::run(
	//			restinio::on_thread_pool<tentris_restinio_traits>(cfg.threads)
	//					.max_parallel_connections(cfg.threads)
	//					.address("0.0.0.0")
	//					.port(cfg.port)
	//					.request_handler(std::move(router))
	//					.handle_request_timeout(cfg.timeout)
	//					.write_http_response_timelimit(cfg.timeout));
	//	log("Shutdown successful.");
	return EXIT_SUCCESS;
}
