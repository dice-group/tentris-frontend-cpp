#ifndef TENTRIS_SERDPARSER2_H
#define TENTRIS_SERDPARSER2_H

#include <Dice/RDF/Term.hpp>
#include <Dice/RDF/Triple.hpp>
#include <Dice/hash/DiceHash.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <serd-0/serd/serd.h>

#include "tentris/store/RDF/TermStore.hpp"
#include "tentris/tensor/BoolHypertrie.hpp"
#include "tentris/util/LogHelper.hpp"


#include <robin_hood.h>

#include <utility>

namespace tentris::store::rdf {

	class SerdParser2 {

		using Triple = Dice::rdf::Triple;
		using Term = Dice::rdf::Term;
		using BNode = Dice::rdf::BNode;
		using Literal = Dice::rdf::Literal;
		using URIRef = Dice::rdf::URIRef;

		using prefixes_map_type = robin_hood::unordered_map<std::string, std::string>;


		struct SerdHandle {
			prefixes_map_type prefixes{};
			tensor::HypertrieBulkInserter &bulk_inserter;
			TermStore &term_index;
			bool done = false;
		};

		static inline SerdStatus on_base(SerdHandle *handle, const SerdNode *uri) {
			handle->prefixes[""] = std::string((char *) (uri->buf), uri->n_bytes);
			return SERD_SUCCESS;
		}

		static inline SerdStatus on_prefix(SerdHandle *handle, const SerdNode *name, const SerdNode *uri) {
			handle->prefixes[std::string((char *) (name->buf), name->n_bytes)] = std::string((char *) (uri->buf), uri->n_bytes);
			return SERD_SUCCESS;
		}

		static inline BNode getBNode(const SerdNode *node) {
			auto identifier = std::string(std::string_view{(char *) (node->buf), size_t(node->n_bytes)});
			return BNode(identifier);
		}

		static inline URIRef getURI(const SerdNode *node) {
			auto iri = std::string(std::string_view{(char *) (node->buf), size_t(node->n_bytes)});
			return URIRef(iri);
		}

		static inline URIRef getPrefixedUri(SerdHandle &handle, const SerdNode *node) {
			std::string_view uri_node_view{(char *) (node->buf), size_t(node->n_bytes)};
			auto sep_pos = uri_node_view.find(':');
			std::string_view prefix{uri_node_view.begin(), sep_pos};
			std::string_view suffix{uri_node_view.begin() + (sep_pos + 1), uri_node_view.size() - sep_pos - 1};
			// TODO that is not safe!
			const std::string &long_prefix = handle.prefixes[std::string{prefix}];
			std::string full_string = long_prefix + std::string{suffix};
			return URIRef(full_string);
		}

		static inline Literal getLiteral(const SerdNode *literal, const SerdNode *type_node, const SerdNode *lang_node) {
			std::string literal_value = std::string{(char *) (literal->buf), size_t(literal->n_bytes)};
			if (type_node != nullptr)
				return Literal(literal_value, std::nullopt,
							   std::string{(char *) (type_node->buf), size_t(type_node->n_bytes)});
			else if (lang_node != nullptr)
				return Literal(literal_value, std::string{(char *) (lang_node->buf), size_t(lang_node->n_bytes)},
							   std::nullopt);
			else
				return Literal(literal_value, std::nullopt, std::nullopt);
		};

		static inline SerdStatus on_statement(SerdHandle *handle,
											  [[maybe_unused]] SerdStatementFlags flags,
											  const SerdNode *graph,
											  const SerdNode *subject,
											  const SerdNode *predicate,
											  const SerdNode *object,
											  const SerdNode *object_datatype,
											  const SerdNode *object_lang) {
			Dice::rdf::Triple triple;
			if (graph != nullptr) {
				std::cerr << "WARNING: File uses graph but graphs are not yet supported." << std::endl;
			}

			switch (subject->type) {
				case SERD_CURIE:
					triple.subject() = getPrefixedUri(*handle, subject);
					break;
				case SERD_URI:
					triple.subject() = getURI(subject);
					break;
				case SERD_BLANK: {
					triple.subject() = getBNode(subject);
				} break;
				default:
					return SERD_ERR_BAD_SYNTAX;
			}

			switch (predicate->type) {
				case SERD_CURIE:
					triple.predicate() = getPrefixedUri(*handle, predicate);
					break;
				case SERD_URI:
					triple.predicate() = getURI(predicate);
					break;
				default:
					return SERD_ERR_BAD_SYNTAX;
			}

			switch (object->type) {
				case SERD_CURIE:
					triple.object() = getPrefixedUri(*handle, object);
					break;
				case SERD_LITERAL:
					triple.object() = getLiteral(object, object_datatype, object_lang);
					break;
				case SERD_BLANK:
					triple.object() = getBNode(object);
					break;
				case SERD_URI:
					triple.object() = getURI(object);
					break;
				default:
					return SERD_ERR_BAD_SYNTAX;
			}
			using RawEntry = tensor::HypertrieBulkInserter::RawEntry<3>;
			if (not triple.subject().isLiteral() and triple.predicate().isURIRef()) {
				auto subject_id = handle->term_index[triple.subject()];
				auto predicate_id = handle->term_index[triple.predicate()];
				auto object_id = handle->term_index[triple.object()];
				handle->bulk_inserter.add(RawEntry{{{subject_id, predicate_id, object_id}}});
				return SERD_SUCCESS;
			} else {
				std::cerr << "Subject or predicate of the triple have a term type that is not allowed there." << std::endl;
				// throw std::invalid_argument{"Subject or predicate of the triple have a term type that is not allowed there."};
				return SERD_SUCCESS;
			}
		}

		static inline SerdStatus on_end([[maybe_unused]] SerdHandle *handle, [[maybe_unused]] const SerdNode *node) {
			handle->done = true;
			return SERD_FAILURE;
		}


	public:
		static void inline parse(tensor::BoolHypertrie &hypertrie, const std::string &path, size_t bulkSize, TermStore &term_index) {

			tensor::HypertrieBulkInserter bulk_inserter{hypertrie, bulkSize,
														[]([[maybe_unused]] size_t processed_entries,
														   [[maybe_unused]] size_t inserted_entries,
														   [[maybe_unused]] size_t hypertrie_size_after) -> void {
															logging::logDebug(fmt::format("{:>10.3} mio triples processed in this batch.", double(processed_entries) / 1'000'000));
															logging::logDebug(fmt::format("{:>10.3} mio triples inserted in this batch.", double(inserted_entries) / 1'000'000));
															logging::logDebug(fmt::format("{:>10.3} mio triples in storage.", (double(hypertrie_size_after) / 1'000'000)));
														}};
			SerdHandle serd_handle{
					.prefixes = {},
					.bulk_inserter = bulk_inserter,
					.term_index = term_index};

//			FILE *file = fopen(path.c_str(), "r");
			SerdReader *reader = serd_reader_new(SERD_TURTLE, (void *) &serd_handle,
												 nullptr,
												 reinterpret_cast<SerdBaseSink>(&on_base),
												 reinterpret_cast<SerdPrefixSink>(&on_prefix),
												 reinterpret_cast<SerdStatementSink>(&on_statement),
												 reinterpret_cast<SerdEndSink>(&on_end));
			serd_reader_read_file(reader, reinterpret_cast<const uint8_t *>(path.c_str()));
//			SerdStatus status = serd_reader_start_stream(reader, file, nullptr, false);
//			if (status == SERD_SUCCESS) {
//				do
//					status = serd_reader_read_chunk(reader);
//				while (not serd_handle.done and (status == SERD_SUCCESS or status == SERD_FAILURE));
////				serd_reader_end_stream(reader);
//			}
			serd_reader_free(reader);
//			fclose(file);
		}
	};
}// namespace tentris::store::rdf

#endif//TENTRIS_SERDPARSER2_H
