#ifndef TENTRIS_STORE_TRIPLESTORE
#define TENTRIS_STORE_TRIPLESTORE

#include <dice/rdf-tensor/Query.hpp>
#include <dice/rdf-tensor/RDFTensor.hpp>

#include <dice/triple-store/SerdLoad.hpp>

#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>

namespace dice::triple_store {

	class TripleStore {

		using HypertrieContext = rdf_tensor::HypertrieContext;
		using HypertrieContext_ptr = rdf_tensor::HypertrieContext_ptr;
		using HypertrieBulkInserter = rdf_tensor::HypertrieBulkInserter;
		using BoolHypertrie = rdf_tensor::BoolHypertrie;
		using const_BoolHypertrie = rdf_tensor::const_BoolHypertrie;
		using Key = rdf_tensor::Key;
		using htt_t = rdf_tensor::htt_t;

	public:
		using allocator_type = rdf_tensor::allocator_type;
		using Entry = rdf_tensor::Entry;

	private:
		HypertrieContext context_;
		BoolHypertrie hypertrie_;
		BoolHypertrie true_scalar_;
		BoolHypertrie false_scalar_;

	public:
		explicit TripleStore(allocator_type const &allocator)
			: context_(allocator),
			  hypertrie_(3, HypertrieContext_ptr(&context_)),
			  true_scalar_(0, HypertrieContext_ptr(&context_)),
			  false_scalar_(0, HypertrieContext_ptr(&context_)) { true_scalar_.set({}, true); }

		[[nodiscard]] BoolHypertrie const &get_hypertrie() const {
			return hypertrie_;
		}

		[[nodiscard]] BoolHypertrie const &get_true_scalar() const {
			return true_scalar_;
		}

		[[nodiscard]] BoolHypertrie const &get_false_scalar() const {
			return false_scalar_;
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

		bool contains(const rdf4cpp::rdf::Statement &statement) {
			return hypertrie_[Key{statement.subject(), statement.predicate(), statement.object()}];
		}

		[[nodiscard]] size_t size() const {
			return hypertrie_.size();
		}
	};
};    // namespace dice::triple-store
#endif//TENTRIS_STORE_TRIPLESTORE
