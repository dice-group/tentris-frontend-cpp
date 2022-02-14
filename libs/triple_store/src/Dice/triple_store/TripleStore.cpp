#include "TripleStore.hpp"

#include <serd-0/serd/serd.h>

namespace Dice::triple_store {
	using namespace rdf4cpp::rdf;

	struct SerdHandle {
		robin_hood::unordered_map<std::string, std::string> prefixes;
		sparql2tensor::HypertrieBulkInserter *bulk_inserter{};
	};

	SerdStatus on_base(SerdHandle *handle, const SerdNode *uri) {
		handle->prefixes[""] = std::string((char *) (uri->buf), uri->n_bytes);
		return SERD_SUCCESS;
	}

	SerdStatus on_prefix(SerdHandle *handle, const SerdNode *name, const SerdNode *uri) {
		handle->prefixes[std::string((char *) (name->buf), name->n_bytes)] = std::string((char *) (uri->buf), uri->n_bytes);
		return SERD_SUCCESS;
	}

	BlankNode getBNode(const SerdNode *node) {
		auto identifier = std::string(std::string_view{(char *) (node->buf), size_t(node->n_bytes)});
		return BlankNode(identifier);
	}

	IRI getURI(const SerdNode *node) {
		auto iri = std::string(std::string_view{(char *) (node->buf), size_t(node->n_bytes)});
		return IRI(iri);
	}

	IRI getPrefixedUri(SerdHandle &handle, const SerdNode *node) {
		std::string_view uri_node_view{(char *) (node->buf), size_t(node->n_bytes)};
		auto sep_pos = uri_node_view.find(':');
		std::string_view prefix{uri_node_view.begin(), sep_pos};
		std::string_view suffix{uri_node_view.begin() + (sep_pos + 1), uri_node_view.size() - sep_pos - 1};
		// TODO that is not safe!
		const std::string &long_prefix = handle.prefixes[std::string{prefix}];
		std::string full_string = long_prefix + std::string{suffix};
		return IRI(full_string);
	}

	Literal getLiteral(const SerdNode *literal, const SerdNode *type_node, const SerdNode *lang_node) {
		std::string literal_value = std::string{(char *) (literal->buf), size_t(literal->n_bytes)};
		if (type_node != nullptr)
			return {literal_value,
					IRI(std::string{(char *) (type_node->buf), size_t(type_node->n_bytes)})};
		else if (lang_node != nullptr)
			return {literal_value, std::string{(char *) (lang_node->buf), size_t(lang_node->n_bytes)}};
		else
			return Literal{literal_value};
	};

	SerdStatus on_statement(SerdHandle *handle,
							[[maybe_unused]] SerdStatementFlags,
							[[maybe_unused]] const SerdNode *,
							const SerdNode *subject,
							const SerdNode *predicate,
							const SerdNode *object,
							const SerdNode *object_datatype,
							const SerdNode *object_lang) {
		hypertrie::internal::raw::SingleEntry<3, hypertrie::internal::raw::Hypertrie_core_t<sparql2tensor::tr>> entry;

		switch (subject->type) {
			case SERD_CURIE:
				entry.key()[0] = getPrefixedUri(*handle, subject);
				break;
			case SERD_URI:
				entry.key()[0] = getURI(subject);
				break;
			case SERD_BLANK: {
				entry.key()[0] = getBNode(subject);
			} break;
			default:
				return SERD_ERR_BAD_SYNTAX;
		}

		switch (predicate->type) {
			case SERD_CURIE:
				entry.key()[1] = getPrefixedUri(*handle, predicate);
				break;
			case SERD_URI:
				entry.key()[1] = getURI(predicate);
				break;
			default:
				return SERD_ERR_BAD_SYNTAX;
		}

		switch (object->type) {
			case SERD_CURIE:
				entry.key()[2] = getPrefixedUri(*handle, object);
				break;
			case SERD_LITERAL:
				entry.key()[2] = getLiteral(object, object_datatype, object_lang);
				break;
			case SERD_BLANK:
				entry.key()[2] = getBNode(object);
				break;
			case SERD_URI:
				entry.key()[2] = getURI(object);
				break;
			default:
				return SERD_ERR_BAD_SYNTAX;
		}
		handle->bulk_inserter->add(entry);
		return SERD_SUCCESS;
	}

	SerdStatus on_end([[maybe_unused]] SerdHandle *handle, [[maybe_unused]] const SerdNode *node) {
		return SERD_SUCCESS;
	}

	TripleStore::TripleStore(metall::manager::allocator_type<std::byte> allocator)
		: context_(allocator), hypertrie_(3, &context_) {}

	sparql2tensor::BoolHypertrie const &TripleStore::get_hypertrie() const {
		return hypertrie_;
	}

	void TripleStore::load_ttl(const std::string &file_path, uint32_t bulk_size, sparql2tensor::HypertrieBulkInserter::BulkInserted_callback const &call_back) {
		sparql2tensor::HypertrieBulkInserter bulk_inserter{hypertrie_, bulk_size, call_back};
		SerdHandle serd_handle{.prefixes = {}, .bulk_inserter = &bulk_inserter};

		SerdReader *reader = serd_reader_new(SERD_TURTLE, (void *) &serd_handle,
											 nullptr,
											 reinterpret_cast<SerdBaseSink>(on_base),
											 reinterpret_cast<SerdPrefixSink>(on_prefix),
											 reinterpret_cast<SerdStatementSink>(on_statement),
											 reinterpret_cast<SerdEndSink>(on_end));
		serd_reader_read_file(reader, reinterpret_cast<const uint8_t *>(file_path.c_str()));
		serd_reader_free(reader);
	}

	void TripleStore::add_statement(const rdf4cpp::rdf::Statement &statement) {
		hypertrie_.set(sparql2tensor::Key{statement.subject(), statement.predicate(), statement.object()}, true);
	}

	std::generator<sparql2tensor::EinsumEntry<sparql2tensor::COUNTED_t> const &>
	TripleStore::query(sparql2tensor::SPARQLQuery query, std::chrono::steady_clock::time_point endtime) {
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
				if (operand.empty())
					co_return;
				else
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
			for (auto const &entry : Dice::query::Evaluation::evaluate<sparql2tensor::const_BoolHypertrie::tr>(q))
				co_yield entry;
		}
	}

	size_t TripleStore::count(sparql2tensor::SPARQLQuery const &query, std::chrono::steady_clock::time_point endtime) {
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

	bool TripleStore::ask(sparql2tensor::SPARQLQuery query, std::chrono::steady_clock::time_point endtime) {
		if (query.triple_patterns_.size() == 1) {// O(1)
			auto slice_key = query.get_slice_keys()[0];
			if (slice_key.get_fixed_depth() == 3)
				return std::get<bool>(get_hypertrie()[slice_key]);
			else
				return not std::get<sparql2tensor::const_BoolHypertrie>(get_hypertrie()[slice_key]).empty();
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

	bool TripleStore::contains(const rdf4cpp::rdf::Statement &statement) {
		return hypertrie_[sparql2tensor::Key{statement.subject(), statement.predicate(), statement.object()}];
	}

	size_t TripleStore::size() const {
		return hypertrie_.size();
	}
}// namespace Dice::triple_store