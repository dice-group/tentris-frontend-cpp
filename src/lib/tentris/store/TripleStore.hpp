#ifndef TENTRIS_STORE_TRIPLESTORE
#define TENTRIS_STORE_TRIPLESTORE

#include "tentris/store/RDF/SerdParser2.hpp"
#include "tentris/store/RDF/TermStore.hpp"
#include "tentris/tensor/BoolHypertrie.hpp"
#include "tentris/util/LogHelper.hpp"

#include <itertools.hpp>
#include <rdf4cpp/rdf.hpp>

#include <optional>
#include <string>
#include <vector>


namespace tentris::store {

	class TripleStore {
		using BoolHypertrie = ::tentris::tensor::BoolHypertrie;
		using const_BoolHypertrie = ::tentris::tensor::const_BoolHypertrie;
		using Term = rdf4cpp::rdf::Node;
		using BNode = rdf4cpp::rdf::BlankNode;
		using Literal = rdf4cpp::rdf::Literal;
		using URIRef = rdf4cpp::rdf::IRI;
		using Triple = rdf4cpp::rdf::Statement;
		using TriplePattern = rdf4cpp::rdf::query::TriplePattern;
		using Variable = rdf4cpp::rdf::query::Variable;

		using TermStore = tentris::store::rdf::TermStore;
		TermStore termIndex{};

		BoolHypertrie trie{3};

	public:
		TermStore &getTermIndex() {
			return termIndex;
		}

		const TermStore &getTermIndex() const {
			return termIndex;
		}

		const_BoolHypertrie getBoolHypertrie() const {
			return trie;
		}

		void bulkloadRDF(const std::string &file_path, size_t bulk_size = 1'000'000) {
			rdf::SerdParser2::parse(trie, file_path, bulk_size, termIndex);
		}

		void add(const std::tuple<Term, Term, Term> &triple) {
			add(std::get<0>(triple), std::get<1>(triple), std::get<2>(triple));
		}

		std::variant<const_BoolHypertrie, bool> resolveTriplePattern(TriplePattern tp) {
			using namespace ::tentris::tensor;

			auto slice_count = 0;
			for (const auto &entry : tp)
				if (entry.is_variable())
					++slice_count;

			SliceKey slice_key(3, std::nullopt);
			for (const auto &[pos, entry] : iter::enumerate(tp)) {
				if (not entry.is_variable()) {
					try {
						auto term = termIndex.get(entry);
						slice_key[pos] = term;
					} catch ([[maybe_unused]] std::out_of_range &exc) {
						// a keypart was not in the index so the result is zero anyways.
						return (slice_count > 0)
									   ? std::variant<const_BoolHypertrie, bool>{
												 const_BoolHypertrie()}
									   : std::variant<const_BoolHypertrie, bool>{false};
					}
				}
			}
			return trie[slice_key];
		}

		inline void
		add(Term subject, Term predicate, Term object) {
			if (not subject.is_literal() and predicate.is_iri()) {
				auto subject_id = termIndex[std::move(subject)];
				auto predicate_id = termIndex[std::move(predicate)];
				auto object_id = termIndex[std::move(object)];
				trie.set({subject_id, predicate_id, object_id}, true);
			} else
				throw std::invalid_argument{
						"Subject or predicate of the triple have a term type that is not allowed there."};
		}

		bool contains(std::tuple<Term, Term, Term> triple) {
			using namespace ::tentris::tensor;
			auto subject = termIndex.find(std::get<0>(triple));
			auto predicate = termIndex.find(std::get<1>(triple));
			auto object = termIndex.find(std::get<2>(triple));
			if (subject and predicate and object) {
				Key key{subject, predicate, object};
				return trie[key];
			}
			return false;
		}

		[[nodiscard]] size_t size() const {
			return trie.size();
		}
	};
};    // namespace tentris::store
#endif//TENTRIS_STORE_TRIPLESTORE
