#ifndef TENTRIS_STORE_TRIPLESTORE
#define TENTRIS_STORE_TRIPLESTORE

#include <Dice/rdf_tensor/RDFTensor.hpp>
#include <Dice/rdf_tensor/RDFEinsum.hpp>

#include <Dice/sparql2tensor/SPARQLQuery.hpp>

#include <Dice/triple_store/SerdLoad.hpp>

#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>

namespace Dice::triple_store {
	class TripleStore {

		using HypertrieContext = rdf_tensor::HypertrieContext;
		using HypertrieContext_ptr = rdf_tensor::HypertrieContext_ptr;
		using HypertrieBulkInserter = rdf_tensor::HypertrieBulkInserter;
		using BoolHypertrie = rdf_tensor::BoolHypertrie;
		using const_BoolHypertrie = rdf_tensor::const_BoolHypertrie;
		using Key = rdf_tensor::Key;
		using htt_t = rdf_tensor::htt_t;
		using UncountedEinsumEntry = rdf_tensor::UncountedEinsumEntry;

	public:
		using allocator_type = rdf_tensor::allocator_type;
		using EinsumEntry = rdf_tensor::EinsumEntry;

	private:

		HypertrieContext context_;
		BoolHypertrie hypertrie_;

	public:

		explicit TripleStore(allocator_type const &allocator)
			: context_(allocator),
			  hypertrie_(3, HypertrieContext_ptr(&context_)) {}

		[[nodiscard]] BoolHypertrie const &get_hypertrie() const {
			return hypertrie_;
		}

		void load_ttl(
				const std::string &file_path,
				uint32_t bulk_size = 1'000'000,
				HypertrieBulkInserter::BulkInserted_callback const &call_back = [](size_t, size_t, size_t) -> void {}) {
			HypertrieBulkInserter bulk_inserter{hypertrie_, bulk_size, call_back};
			AddTripleCallback_function add_entry_callback =
					[&bulk_inserter](rdf4cpp::rdf::Node subj, rdf4cpp::rdf::Node pred, rdf4cpp::rdf::Node obj) noexcept -> void {
				hypertrie::internal::raw::SingleEntry<3, htt_t> entry{{subj, pred, obj}};
				bulk_inserter.add(entry);
			};
			serd_load(file_path, add_entry_callback);
		}

		void add_statement(const rdf4cpp::rdf::Statement &statement) {
			static_assert(sizeof(statement.subject()) == sizeof(uint64_t));
			static_assert(sizeof(Key::value_type) == sizeof(uint64_t));
			Key key{statement.subject(), statement.predicate(), statement.object()};
			hypertrie_.set(key, true);
		}

		std::generator<EinsumEntry const &> query(
				sparql2tensor::SPARQLQuery query,
				std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) {
			std::vector<const_BoolHypertrie> operands;
			for (auto const &slice_key : query.get_slice_keys()) {
				auto slice_result = hypertrie_[slice_key];
				if (slice_key.get_fixed_depth() == 3) {
					auto entry_exists = std::get<bool>(slice_result);
					if (not entry_exists)
						co_return;
				} else {
					auto operand = std::get<const_BoolHypertrie>(slice_result);
					if (operand.empty())
						co_return;
					else
						operands.push_back(std::move(operand));
				}
			}
			auto subscript = query.get_subscript();
			if (query.distinct_) {
				EinsumEntry entry;
				entry.key().resize(query.projected_variables_.size());
				for (auto const &distinct_entry : ::Dice::einsum::einsum<rdf_tensor::DISTINCT_t , htt_t>(subscript, operands, endtime)) {
					std::copy(distinct_entry.key().begin(), distinct_entry.key().end(), entry.key().begin());
					co_yield entry;
				}
			} else {
				for (auto const &entry : ::Dice::einsum::einsum<rdf_tensor::COUNTED_t , htt_t>(subscript, operands, endtime))
					co_yield entry;
			}
		}

		size_t count(sparql2tensor::SPARQLQuery query,
					 std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) {
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

		bool ask(sparql2tensor::SPARQLQuery query,
				 std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) {
			if (query.triple_patterns_.size() == 1) {// O(1)
				auto slice_key = query.get_slice_keys()[0];
				if (slice_key.get_fixed_depth() == 3)
					return std::get<bool>(get_hypertrie()[slice_key]);
				else
					return not std::get<const_BoolHypertrie>(get_hypertrie()[slice_key]).empty();
			} else {
				query.projected_variables_.clear();
				query.project_all_variables_ = false;
				query.distinct_ = true;
				for ([[maybe_unused]] auto const &_ : this->query(query, endtime)) {
					return true;
					break;
				}
				return false;
			}
		}

		bool contains(const rdf4cpp::rdf::Statement &statement) {
			return hypertrie_[Key{statement.subject(), statement.predicate(), statement.object()}];
		}


		[[nodiscard]] size_t size() const {
			return hypertrie_.size();
		}
	};
};    // namespace Dice::triple_store
#endif//TENTRIS_STORE_TRIPLESTORE
