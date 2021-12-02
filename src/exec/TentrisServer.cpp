
#include <restinio/all.hpp>
#include <tentris/http/SparqlEndpoint.hpp>
#include <tentris/store/AtomicTripleStore.hpp>
#include <tentris/store/TripleStore.hpp>
#include <tentris/store/config/AtomicTripleStoreConfig.cpp>

#include <fmt/format.h>

#include "VersionStrings.hpp"
#include "config/ServerConfig.hpp"

#include <chrono>
#include <csignal>
#include <filesystem>


void bulkload(const std::string &triple_file, size_t bulksize) {
	namespace fs = std::filesystem;
	using namespace fmt::literals;
	using namespace tentris::logging;

	// log the starting time and print resource usage information
	auto loading_start_time = log_health_data();

	if (fs::is_regular_file(triple_file)) {
		log("nt-file: {} loading ..."_format(triple_file));
		::tentris::store::AtomicTripleStore::getInstance().bulkloadRDF(triple_file, bulksize);
	} else {
		log("nt-file {} was not found."_format(triple_file));
		log("Exiting ...");
		std::exit(EXIT_FAILURE);
	}
	// log the end time and print resource usage information
	auto loading_end_time = log_health_data();
	// log the time it tool to load the file
	log_duration(loading_start_time, loading_end_time);
}

struct tentris_restinio_traits : public restinio::traits_t<
										 restinio::null_timer_manager_t,
#ifdef DEBUG
										 restinio::shared_ostream_logger_t,
#else
										 restinio::null_logger_t,
#endif
										 restinio::router::express_router_t<>> {
	static constexpr bool use_connection_count_limiter = true;
};


int main(int argc, char *argv[]) {
	using namespace tentris::http;
	using namespace tentris::store::config;
	using namespace fmt::literals;
	using namespace tentris::logging;

	ServerConfig cfg{argc, argv};

	init_logging(cfg.logstdout, cfg.logfile, cfg.logfiledir, cfg.loglevel);

	log("Running {} with {}"_format(tentris_version_string, hypertrie_version_string));

	auto &store_cfg = AtomicTripleStoreConfig::getInstance();
	store_cfg.rdf_file = cfg.rdf_file;
	store_cfg.timeout = cfg.timeout;
	store_cfg.cache_size = cfg.cache_size;
	store_cfg.threads = cfg.threads;

	// bulkload file
	if (not cfg.rdf_file.empty()) {
		bulkload(cfg.rdf_file, cfg.bulksize);
	} else {
		log("No file loaded.");
	}

	// create endpoint
	using namespace restinio;
	auto router = std::make_unique<router::express_router_t<>>();
	router->http_get(
			R"(/sparql)",
			tentris::http::sparql_endpoint::SparqlEndpoint<restinio::restinio_controlled_output_t>{});
	router->http_get(
			R"(/stream)",
			tentris::http::sparql_endpoint::SparqlEndpoint<restinio::chunked_output_t>{});
	router->http_get(
			R"(/count)",
			[&](restinio::request_handle_t req, auto const &) -> restinio::request_handling_status_t {
				using namespace ::std::chrono;
				using namespace ::tentris::logging;
				using namespace ::tentris::tensor;
				using AtomicQueryExecutionCache = ::tentris::store::AtomicQueryExecutionCache;
				using AtomicTripleStoreConfig = ::tentris::store::config::AtomicTripleStoreConfig;
				using QueryExecutionPackage = ::tentris::store::cache::QueryExecutionPackage;
				using SelectModifier = Dice::sparql::Nodes::QueryNodes::SelectNodes::SelectModifier;

				auto start_time = steady_clock::now();
				log("count request started.");
				auto start_memory = get_memory_usage();
				logDebug("ram: {:d} kB"_format(start_memory));
				auto timeout = start_time + AtomicTripleStoreConfig::getInstance().timeout;
				std::shared_ptr<QueryExecutionPackage> query_package;
				std::string query_string{};
				try {
					const auto query_params = restinio::parse_query<restinio::parse_query_traits::javascript_compatible>(
							req->header().query());
					query_string = std::string(query_params["query"]);
					log("query: {}"_format(query_string));
					// check if there is actually an query
					try {
						query_package = AtomicQueryExecutionCache::getInstance()[query_string];
					} catch (const std::invalid_argument &exc) {
						logDebug(exc.what());
						return req->create_response(status_bad_request()).set_body("Query could not be parsed.").done();
					}
					size_t count = 0;
					try {

						if (query_package->getSelectModifier() == SelectModifier::DISTINCT) {
							for ([[maybe_unused]] auto const &entry : Dice::einsum::einsum<DISTINCT_t, tr>(query_package->getSubscript(), query_package->getOperands(), timeout))
								++count;
						} else {
							for (auto const &entry : Dice::einsum::einsum<DISTINCT_t, tr>(query_package->getSubscript(), query_package->getOperands(), timeout))
								count += entry.value();
						}

						log("result_count: {}"_format(count));
						return req->create_response().set_body(fmt::format("{}", count)).done();

					} catch (Dice::einsum::TimeoutException const &ex) {
						return req->create_response(status_request_time_out())
								.set_body(fmt::format("Timed out after {} s and counting {} results.",
													  duration_cast<seconds>(AtomicTripleStoreConfig::getInstance().timeout).count()))
								.done();
					}
				} catch (const std::exception &exc) {
					// if the execution of the query should fail return an internal server error
					logDebug(exc.what());
					return req->create_response(status_bad_request()).set_body("?query parameter missing or faulty.").done();
				}
			});

	router->http_get(
			R"(/ask)",
			[&](restinio::request_handle_t req, auto const &) -> restinio::request_handling_status_t {
				using namespace ::std::chrono;
				using namespace ::tentris::logging;
				using namespace ::tentris::tensor;
				using AtomicQueryExecutionCache = ::tentris::store::AtomicQueryExecutionCache;
				using AtomicTripleStoreConfig = ::tentris::store::config::AtomicTripleStoreConfig;
				using QueryExecutionPackage = ::tentris::store::cache::QueryExecutionPackage;

				auto start_time = steady_clock::now();
				log("ask request started.");
				auto start_memory = get_memory_usage();
				logDebug("ram: {:d} kB"_format(start_memory));
				auto timeout = start_time + AtomicTripleStoreConfig::getInstance().timeout;
				std::shared_ptr<QueryExecutionPackage> query_package;
				std::string query_string{};
				try {
					const auto query_params = restinio::parse_query<restinio::parse_query_traits::javascript_compatible>(
							req->header().query());
					query_string = std::string(query_params["query"]);
					log("query: {}"_format(query_string));
					// check if there is actually an query
					try {
						query_package = AtomicQueryExecutionCache::getInstance()[query_string];
					} catch (const std::invalid_argument &exc) {
						logDebug(exc.what());
						return req->create_response(status_bad_request()).set_body("Query could not be parsed.").done();
					}
					try {
						auto ask_subscript = std::make_shared<Dice::einsum::Subscript>(query_package->getSubscript()->getRawSubscript().operands, Subscript::ResultSc{});
						bool ask = false;
						for (auto const &entry : Dice::einsum::einsum<DISTINCT_t, tr>(query_package->getSubscript(), query_package->getOperands(), timeout)) {
							if (entry.value()) {
								ask = true;
								break;
							}
						}
						log("ask: {}"_format(ask));
						return req->create_response().set_body(fmt::format("{}", ask)).done();

					} catch (Dice::einsum::TimeoutException const &ex) {
						return req->create_response(status_request_time_out())
								.set_body(fmt::format("Timed out after {} s.",
													  duration_cast<seconds>(AtomicTripleStoreConfig::getInstance().timeout).count()))
								.done();
					}
				} catch (const std::exception &exc) {
					// if the execution of the query should fail return an internal server error
					logDebug(exc.what());
					return req->create_response(status_bad_request()).set_body("?query parameter missing or faulty.").done();
				}
			});

	router->non_matched_request_handler(
			[](auto req) -> restinio::request_handling_status_t {
				return req->create_response(restinio::status_not_found()).connection_close().done();
			});

	// Launching a server with custom traits.

	log("SPARQL endpoint serving sparkling linked data treasures on {} threads at http://0.0.0.0:{}/sparql?query="_format(cfg.threads, cfg.port));

	restinio::run(
			restinio::on_thread_pool<tentris_restinio_traits>(cfg.threads)
			        .max_parallel_connections(cfg.threads)
					.address("0.0.0.0")
					.port(cfg.port)
					.request_handler(std::move(router))
					.handle_request_timeout(cfg.timeout)
					.write_http_response_timelimit(cfg.timeout));
	log("Shutdown successful.");
	return EXIT_SUCCESS;
}
