#ifndef TENTRIS_STORE_TRIPLESTORE
#define TENTRIS_STORE_TRIPLESTORE

#include <Dice/sparql2tensor/BoolHypertrie.hpp>
#include <Dice/sparql2tensor/SPARQLQuery.hpp>
#include <Dice/triple_store/SerdLoad.hpp>

#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>

namespace Dice::triple_store {
	template<typename Allocator = std::allocator<std::byte>>
	class TripleStore {

		sparql2tensor::HypertrieContext context_;
		sparql2tensor::BoolHypertrie hypertrie_;

	public:

		explicit TripleStore(Allocator const &allocator = Allocator())
			: context_(allocator),
			  hypertrie_(3,
						 [&]() { if constexpr(std::is_same_v<typename std::allocator_traits<Allocator>::void_pointer, void*>) return &context_; else return Dice::hypertrie::HypertrieContext_ptr<sparql2tensor::tr>(&context_); }()) {}

		[[nodiscard]] sparql2tensor::BoolHypertrie const &get_hypertrie() const {
			return hypertrie_;
		}

		void load_ttl(
				const std::string &file_path,
				uint32_t bulk_size = 1'000'000,
				sparql2tensor::HypertrieBulkInserter::BulkInserted_callback const &call_back = [](size_t, size_t, size_t) -> void {}) {
			sparql2tensor::HypertrieBulkInserter bulk_inserter{hypertrie_, bulk_size, call_back};
			AddTripleCallback_function add_entry_callback =
					[&bulk_inserter](rdf4cpp::rdf::Node subj, rdf4cpp::rdf::Node pred, rdf4cpp::rdf::Node obj) noexcept -> void {
				hypertrie::internal::raw::SingleEntry<3, hypertrie::internal::raw::Hypertrie_core_t<sparql2tensor::tr>> entry{{subj, pred, obj}};
				bulk_inserter.add(entry);
			};
			serd_load(file_path, add_entry_callback);
		}

		void add_statement(const rdf4cpp::rdf::Statement &statement) {
			static_assert(sizeof(statement.subject()) == sizeof(uint64_t));
			static_assert(sizeof(sparql2tensor::Key::value_type) == sizeof(uint64_t));
			sparql2tensor::Key key{statement.subject(), statement.predicate(), statement.object()};
			hypertrie_.set(key, true);
		}

		std::generator<sparql2tensor::EinsumEntry<sparql2tensor::COUNTED_t> const &>
		query(sparql2tensor::SPARQLQuery query, std::chrono::steady_clock::time_point endtime) {
			std::vector<sparql2tensor::const_BoolHypertrie> operands;
			for (auto const &slice_key : query.get_slice_keys()) {
				auto slice_result = hypertrie_[slice_key];
				if (slice_key.get_fixed_depth() == 3) {
					auto entry_exists = std::get<bool>(slice_result);
					sparql2tensor::BoolHypertrie ht_0{0, &context_};
					if (entry_exists)
						ht_0.set({}, true);
					operands.push_back(ht_0);
				} else {
					auto operand = std::get<sparql2tensor::const_BoolHypertrie>(slice_result);
					operands.push_back(std::move(operand));
				}
			}
			std::vector<char> proj_vars_id{};
			for (auto const &proj_var : query.projected_variables_) {
				proj_vars_id.push_back(query.var_to_id_[proj_var]);
			}
			sparql2tensor::Query q{query.get_odg(), operands, proj_vars_id};
			if (query.distinct_) {
				sparql2tensor::EinsumEntry<sparql2tensor::COUNTED_t> entry;
				entry.key().resize(query.projected_variables_.size());
				for (auto const &distinct_entry : Dice::query::Evaluation::evaluate<sparql2tensor::const_BoolHypertrie::tr, true>(q)) {
					std::copy(distinct_entry.key().begin(), distinct_entry.key().end(), entry.key().begin());
					co_yield entry;
				}
			} else {
				for (auto const &entry : Dice::query::Evaluation::evaluate<sparql2tensor::const_BoolHypertrie::tr>(q)) {
					co_yield entry;
				}
			}
		}

		size_t count(sparql2tensor::SPARQLQuery const &query, std::chrono::steady_clock::time_point endtime) {
			using namespace sparql2tensor;
			if (query.triple_patterns_.size() == 1) {// O(1)
				auto slice_key = query.get_slice_keys()[0];
				if (slice_key.get_fixed_depth() == 3)
					return (size_t) std::get<bool>(get_hypertrie()[slice_key]);
				else
					return std::get<const_BoolHypertrie>(get_hypertrie()[slice_key]).size();
			} else {
				size_t count = 0;
				for (auto const &entry : this->query(query, endtime))
					count += entry.value();
				return count;
			}
		}

		bool ask(sparql2tensor::SPARQLQuery query, std::chrono::steady_clock::time_point endtime) {
			std::vector<sparql2tensor::const_BoolHypertrie> operands;
			for (auto const &slice_key : query.get_slice_keys()) {
				auto slice_result = hypertrie_[slice_key];
				if (slice_key.get_fixed_depth() == 3) {
					auto entry_exists = std::get<bool>(slice_result);
					sparql2tensor::BoolHypertrie ht_0{0, &context_};
					if (entry_exists)
						ht_0.set({}, true);
					operands.push_back(ht_0);
				} else {
					auto operand = std::get<sparql2tensor::const_BoolHypertrie>(slice_result);
					operands.push_back(std::move(operand));
				}
			}
			std::vector<char> proj_vars_id{};
			for (auto const &proj_var : query.projected_variables_) {
				proj_vars_id.push_back(query.var_to_id_[proj_var]);
			}
			sparql2tensor::Query q{query.get_odg(), operands, proj_vars_id};
			return Dice::query::Evaluation::evaluate_ask<sparql2tensor::const_BoolHypertrie::tr>(q);
		}

		bool contains(const rdf4cpp::rdf::Statement &statement) {
			return hypertrie_[sparql2tensor::Key{statement.subject(), statement.predicate(), statement.object()}];
		}


		[[nodiscard]] size_t size() const {
			return hypertrie_.size();
		}
	};
};    // namespace Dice::triple_store
#endif//TENTRIS_STORE_TRIPLESTORE
