#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>

#include <csv.hpp>

#include <tentris/store/RDF/TermStore.hpp>
#include <tentris/store/RDF/SerdParser2.hpp>
#include <tentris/tensor/BoolHypertrie.hpp>
#include <boost/lexical_cast.hpp>
#include <tentris/util/LogHelper.hpp>

namespace tentris::IDs2Hypertrie {
	void writeNodeStatsTSVs(const auto &storage_3_uncompressed, const auto &storage_2_uncompressed,
							const auto &storage_2_sen, const auto &storage_1_uncompressed,
							const auto &storage_1_sen);

	void writeNodeCountComparisonTSVs(const auto &storage_2_uncompressed, const auto &storage_2_sen,
									  const auto &storage_1_uncompressed, const auto &storage_1_sen);

	void loadIDsAndWriteOutStats(const std::string &csv_file_path);
}
int main(int argc, char *argv[]) {
	using namespace fmt::literals;
	if (argc != 2) {
		std::cerr << "Please provide exactly one CSV file with triple IDS only and no headings." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::string csv_file_path{argv[1]};
	if (not std::filesystem::is_regular_file(csv_file_path)) {
		std::cerr << "{} is not a file."_format(csv_file_path) << std::endl;
		exit(EXIT_FAILURE);
	}

	tentris::IDs2Hypertrie::loadIDsAndWriteOutStats(csv_file_path);
}

namespace tentris::IDs2Hypertrie {
	using namespace tentris::store;
	using namespace fmt::literals;
	using namespace std::chrono;
	
	using key_part_type = size_t;
	
	using tr = Dice::hypertrie::Hypertrie_trait<key_part_type,
												bool,
												std::allocator<std::byte>,
												Dice::hypertrie::internal::container::tsl_sparse_map,
												Dice::hypertrie::internal::container::tsl_sparse_set,
												-1>;
	using tri = Dice::hypertrie::internal::raw::Hypertrie_core_t<tr>;
	using BoolHypertrie = Dice::hypertrie::Hypertrie<tr>;
	using const_BoolHypertrie = Dice::hypertrie::const_Hypertrie<tr>;
	using HypertrieBulkInserter = Dice::hypertrie::BulkInserter<tr>;
	using SliceKey = Dice::hypertrie::SliceKey<tr>;
	using Key = Dice::hypertrie::Key<tr>;
	using NonZeroEntry = Dice::hypertrie::NonZeroEntry<tr>;
	
	constexpr static size_t bulk_size = 1'000'000;

	void loadIDsAndWriteOutStats(const std::string &csv_file_path) {
		BoolHypertrie trie(3);

		csv::CSVFormat format;
		format.delimiter('\t').quote(false);


		csv::CSVReader tsv_reader(csv_file_path, format);

		// Iterate through each line and split the content using delimiter
		unsigned long count = 0;
		auto start = steady_clock::now();

		try {
			HypertrieBulkInserter bulk_inserter{trie, 1'000'000,
												[]([[maybe_unused]] size_t processed_entries,
												   [[maybe_unused]] size_t inserted_entries,
												   [[maybe_unused]] size_t hypertrie_size_after) noexcept {
													std::cerr << "{:>10.3} mio triples processed.\n"_format(double(processed_entries) / bulk_size);
													std::cerr << "{:>10.3} mio triples loaded.\n"_format(double(hypertrie_size_after) / bulk_size);
												}};

			for (csv::CSVRow &row: tsv_reader) { // Input iterator
				row[0].get<size_t>();
				using RawEntry = typename HypertrieBulkInserter::template RawEntry<3>;
				
				bulk_inserter.add(RawEntry{{{row[0].get<size_t>(),
								   row[1].get<size_t>(),
								   row[2].get<size_t>()}}});
				++count;
			}

			bulk_inserter.flush();

		} catch (...) {
			throw std::invalid_argument{"A parsing error occurred while parsing {}"_format(csv_file_path)};
		}
		auto end = steady_clock::now();
		auto duration = end - start;


		auto &storage = trie.context()->raw_context().node_storage_;

		const auto &storage_3_fn = storage.nodes<3, Dice::hypertrie::internal::raw::FullNode>().nodes();
		const auto &storage_2_fn = storage.nodes<2, Dice::hypertrie::internal::raw::FullNode>().nodes();
		const auto &storage_1_fn = storage.nodes<1, Dice::hypertrie::internal::raw::FullNode>().nodes();
		const auto &storage_3_sen = storage.nodes<3, Dice::hypertrie::internal::raw::SingleEntryNode>().nodes();
		const auto &storage_2_sen = storage.nodes<2, Dice::hypertrie::internal::raw::SingleEntryNode>().nodes();
		const auto &storage_1_sen = storage.nodes<1, Dice::hypertrie::internal::raw::SingleEntryNode>().nodes();
		
		writeNodeStatsTSVs(storage_3_fn, storage_2_fn, storage_3_sen, storage_1_fn,
						   storage_1_sen);

		writeNodeCountComparisonTSVs(storage_2_fn, storage_2_sen, storage_1_fn,
									 storage_1_sen);


		std::cerr << "## total ## \n"
				  << "triples processed: {}\n"_format(count)
				  << "triples loaded: {}\n"_format(trie.size())
				  << "hypertrie size estimation: {:d} kB\n"_format(tentris::logging::get_memory_usage())
				  << "duration: {} h {} min {}.{:03d} s = {} ms\n"_format(
						  (std::chrono::duration_cast<std::chrono::hours>(duration)).count(),
						  (std::chrono::duration_cast<std::chrono::minutes>(duration) % 60).count(),
						  (std::chrono::duration_cast<std::chrono::seconds>(duration) % 60).count(),
						  (std::chrono::duration_cast<std::chrono::milliseconds>(duration) % 1000).count(),
						  std::chrono::duration_cast<std::chrono::milliseconds>(duration).count())
				  << "# hypertrie stats #\n"
				  << "depth 3 full nodes: {}\n"_format(storage_3_fn.size())
				  << "depth 2 full nodes: {}\n"_format(storage_2_fn.size())
				  << "depth 2 single entry nodes: {}\n"_format(storage_2_sen.size())
				  << "depth 1 full nodes: {}\n"_format(storage_1_fn.size())
				  << "depth 1 single entry nodes: {}\n"_format(storage_1_sen.size());
	}

	void writeNodeCountComparisonTSVs(const auto &storage_2_fn, const auto &storage_2_sen,
									  const auto &storage_1_fn, const auto &storage_1_sen) {
		{
			std::ofstream tsv_depth_2_comp("depth_2_node_count_comparision.tsv");
			auto csv_writer = csv::make_tsv_writer(tsv_depth_2_comp);

			csv_writer << std::make_tuple("hypertrie_type", "full_nodes", "sen_nodes");

			{ // baseline
				size_t fn_nodes = [&]() {
					size_t old_uc = 0;
					for (auto[hash, node] : storage_2_fn)
						old_uc += node->ref_count();
					for (auto[hash, node] : storage_2_sen)
						old_uc += node->ref_count();
					return old_uc;
				}();

				size_t sen_nodes = 0;

				csv_writer << std::make_tuple("baseline", fn_nodes, sen_nodes);
			}

			{ // compression
				size_t fn_nodes = [&]() {
					size_t old_uc = 0;
					for (auto[hash, node] : storage_2_fn)
						old_uc += node->ref_count();
					return old_uc;
				}();

				size_t sen_nodes = [&]() {
					size_t sen_nodes = 0;
					for (auto[hash, node] : storage_2_sen)
						sen_nodes += node->ref_count();
					return sen_nodes;
				}();

				csv_writer << std::make_tuple("compression", fn_nodes, sen_nodes);
			}

			{ // hash
				size_t fn_nodes = storage_2_fn.size() + storage_2_sen.size();

				size_t sen_nodes = 0;

				csv_writer << std::make_tuple("hash", fn_nodes, sen_nodes);
			}

			{ // hash+compression and hash+compression+inline
				size_t fn_nodes = storage_2_fn.size();

				size_t sen_nodes = storage_2_sen.size();

				csv_writer << std::make_tuple("hash+compression", fn_nodes, sen_nodes);
				csv_writer << std::make_tuple("hash+compression+inline", fn_nodes, sen_nodes);
			}
		}

		{
			std::ofstream tsv_depth_1_comp("depth_1_node_count_comparision.tsv");
			auto csv_writer = csv::make_tsv_writer(tsv_depth_1_comp);

			csv_writer << std::make_tuple("hypertrie_type", "full_nodes", "sen_nodes");

			{ // baseline
				size_t sen_depth2_nodes = [&]() {
					size_t depth2nodes = 0;
					for (auto[hash, node] : storage_2_fn)
						depth2nodes += node->ref_count();
					return depth2nodes;
				}();

				size_t depth1_nodes = [&]() {
					size_t fn_nodes = 0;
					for (auto[hash, node] : storage_1_fn)
						fn_nodes += node->ref_count();
					for (auto[hash, node] : storage_1_sen)
						fn_nodes += node->ref_count();
					return fn_nodes;
				}();

				size_t fn_nodes = sen_depth2_nodes + (depth1_nodes / 2);

				size_t sen_nodes = 0;

				csv_writer << std::make_tuple("baseline", fn_nodes, sen_nodes);
			}

			{ // compression
				size_t fn_nodes = [&]() {
					size_t fn_nodes = 0;
					for (auto[hash, node] : storage_1_fn)
						fn_nodes += node->ref_count();
					return fn_nodes;
				}() / 2;

				size_t sen_nodes = [&]() {
					size_t x = 0;
					for (auto[hash, node] : storage_1_sen)
						x += node->ref_count();
					return x;
				}();
				sen_nodes = sen_nodes / 2;

				csv_writer << std::make_tuple("compression", fn_nodes, sen_nodes);
			}

			{ // hash
//				using TensorHash = hypertrie::internal::raw::TensorHash;

				size_t sen_nodes_count = [&]() {
					robin_hood::unordered_set<Dice::hypertrie::internal::raw::RawIdentifier<1, tri>> sen_d1_hashes;

					// add the hashes from depth 1 single entry nodes.
					for (auto[hash, node] : storage_1_sen)
						sen_d1_hashes.insert(hash);

					// break apart the depth 2 single entry nodes and a Hash for each of both key parts
					for (auto[hash, node] : storage_2_sen) {
						sen_d1_hashes.template emplace(Dice::hypertrie::internal::raw::SingleEntry<1, tri>({{node->key()[0]}}));
						sen_d1_hashes.template emplace(Dice::hypertrie::internal::raw::SingleEntry<1, tri>({{node->key()[1]}}));
					}

					return sen_d1_hashes.size();
				}();

				size_t fn_nodes = sen_nodes_count + storage_1_fn.size();

				size_t sen_nodes = 0;

				csv_writer << std::make_tuple("hash", fn_nodes, sen_nodes);
			}

			{ // hash+compression and hash+compression+inline
				size_t fn_nodes = storage_1_fn.size();

				size_t sen_nodes = storage_1_sen.size();

				csv_writer << std::make_tuple("hash+compression", fn_nodes, sen_nodes);
				csv_writer << std::make_tuple("hash+compression+inline", fn_nodes, 0);
			}

		}
	}

	void writeNodeStatsTSVs(const auto &storage_3_fn, const auto &storage_2_fn,
							const auto &storage_2_sen, const auto &storage_1_fn,
							const auto &storage_1_sen) {
		auto node_type_flag = [](const auto &hash) { return (hash.is_sen()) ? "s" : "f"; };

		{
			std::ofstream tsv_depth_3("depth_3_nodes_stats.tsv"); // Can also use ofstream, etc.
			auto csv_writer = csv::make_tsv_writer(tsv_depth_3);


			csv_writer
					<< std::make_tuple("node_type", "node_size", "dimension_1_size", "dimension_2_size",
									   "dimension_3_size",
									   "reference_count");
			for (auto[hash, node] : storage_3_fn) {
				csv_writer << std::make_tuple(node_type_flag(hash), node->size(), node->edges(0).size(),
											  node->edges(1).size(),
											  node->edges(2).size(),
											  node->ref_count());
			}
		}

		{
			std::ofstream tsv_depth_2("depth_2_nodes_stats.tsv"); // Can also use ofstream, etc.
			auto csv_writer = csv::make_tsv_writer(tsv_depth_2);

			csv_writer
					<< std::make_tuple("node_type", "node_size", "dimension_1_size", "dimension_2_size",
									   "reference_count");

			for (auto[hash, node] : storage_2_sen) {
				csv_writer << std::make_tuple(node_type_flag(hash), node->size(), 1, 1, node->ref_count());
			}

			for (auto[hash, node] : storage_2_fn) {
				csv_writer
						<< std::make_tuple(node_type_flag(hash), node->size(), node->edges(0).size(),
										   node->edges(1).size(), node->ref_count());
			}
		}

		{
			std::ofstream tsv_depth_1("depth_1_nodes_stats.tsv"); // Can also use ofstream, etc.
			auto csv_writer = csv::make_tsv_writer(tsv_depth_1);

			csv_writer << std::make_tuple("node_type", "node_size", "dimension_1_size", "reference_count");

			for (auto[hash, node] : storage_1_sen) {
				csv_writer << std::make_tuple(node_type_flag(hash), node->size(), 1, node->ref_count());
			}

			for (auto[hash, node] : storage_1_fn) {
				csv_writer << std::make_tuple(node_type_flag(hash), node->size(), node->edges(0).size(),
											  node->ref_count());
			}
		}
	}


}