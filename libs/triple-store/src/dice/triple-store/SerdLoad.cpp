#include "SerdLoad.hpp"

#include <robin_hood.h>
#include <serd/serd.h>

#include <utility>

using namespace rdf4cpp::rdf;

struct SerdHandle {
	robin_hood::unordered_map<std::string, std::string> prefixes;
	dice::triple_store::AddTripleCallback_function add_triple_callback;
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
	Node rdf_subject;

	switch (subject->type) {
		case SERD_CURIE:
			rdf_subject = getPrefixedUri(*handle, subject);
			break;
		case SERD_URI:
			rdf_subject = getURI(subject);
			break;
		case SERD_BLANK: {
			rdf_subject = getBNode(subject);
		} break;
		default:
			return SERD_ERR_BAD_SYNTAX;
	}

	Node rdf_predicate;

	switch (predicate->type) {
		case SERD_CURIE:
			rdf_predicate = getPrefixedUri(*handle, predicate);
			break;
		case SERD_URI:
			rdf_predicate = getURI(predicate);
			break;
		default:
			return SERD_ERR_BAD_SYNTAX;
	}

	Node rdf_object;

	switch (object->type) {
		case SERD_CURIE:
			rdf_object = getPrefixedUri(*handle, object);
			break;
		case SERD_LITERAL:
			rdf_object = getLiteral(object, object_datatype, object_lang);
			break;
		case SERD_BLANK:
			rdf_object = getBNode(object);
			break;
		case SERD_URI:
			rdf_object = getURI(object);
			break;
		default:
			return SERD_ERR_BAD_SYNTAX;
	}
	handle->add_triple_callback(rdf_subject, rdf_predicate, rdf_object);
	return SERD_SUCCESS;
}

SerdStatus on_statement_no_blank_nodes(SerdHandle *handle,
						[[maybe_unused]] SerdStatementFlags,
						[[maybe_unused]] const SerdNode *,
						const SerdNode *subject,
						const SerdNode *predicate,
						const SerdNode *object,
						const SerdNode *object_datatype,
						const SerdNode *object_lang) {
	Node rdf_subject;

	switch (subject->type) {
		case SERD_CURIE:
			rdf_subject = getPrefixedUri(*handle, subject);
			break;
		case SERD_URI:
			rdf_subject = getURI(subject);
			break;
		default:
			return SERD_ERR_BAD_SYNTAX;
	}

	Node rdf_predicate;

	switch (predicate->type) {
		case SERD_CURIE:
			rdf_predicate = getPrefixedUri(*handle, predicate);
			break;
		case SERD_URI:
			rdf_predicate = getURI(predicate);
			break;
		default:
			return SERD_ERR_BAD_SYNTAX;
	}

	Node rdf_object;

	switch (object->type) {
		case SERD_CURIE:
			rdf_object = getPrefixedUri(*handle, object);
			break;
		case SERD_LITERAL:
			rdf_object = getLiteral(object, object_datatype, object_lang);
			break;
		case SERD_URI:
			rdf_object = getURI(object);
			break;
		default:
			return SERD_ERR_BAD_SYNTAX;
	}
	handle->add_triple_callback(rdf_subject, rdf_predicate, rdf_object);
	return SERD_SUCCESS;
}

SerdStatus on_end([[maybe_unused]] SerdHandle *handle, [[maybe_unused]] const SerdNode *node) {
	return SERD_SUCCESS;
}

SerdStatus reject_base(SerdHandle *, const SerdNode *) {
	return SERD_ERR_BAD_SYNTAX;
}

SerdStatus reject_prefix(SerdHandle *, const SerdNode *, const SerdNode *) {
	return SERD_ERR_BAD_SYNTAX;
}

void dice::triple_store::serd_load(const std::string &file_path, dice::triple_store::AddTripleCallback_function add_triple_callback) {
	SerdHandle serd_handle{.prefixes = {}, .add_triple_callback = std::move(add_triple_callback)};

	SerdReader *reader = serd_reader_new(SERD_TURTLE, (void *) &serd_handle,
										 nullptr,
										 reinterpret_cast<SerdBaseSink>(on_base),
										 reinterpret_cast<SerdPrefixSink>(on_prefix),
										 reinterpret_cast<SerdStatementSink>(on_statement),
										 reinterpret_cast<SerdEndSink>(on_end));
	serd_reader_read_file(reader, reinterpret_cast<const uint8_t *>(file_path.c_str()));
	serd_reader_free(reader);
}

SerdStatus error_sink([[maybe_unused]] void *handle, SerdError const *error) {
	std::ostringstream error_msg;
	error_msg << "Syntax error: " << serd_strerror(error->status) << ". At line: " << error->line << " and position: " << error->col;
	throw std::runtime_error{error_msg.str()};
}

void dice::triple_store::serd_load_delete_data_triples_from_string(std::string_view triples, dice::triple_store::AddTripleCallback_function add_triple_callback) {
	struct Source {
		std::string_view triples;

		static size_t read(void *buf, [[maybe_unused]] size_t size, size_t count, void *voided_self) noexcept {
			assert(size == 1);

			if (count == 0) {
				return 0;
			}

			auto *self = reinterpret_cast<Source *>(voided_self);

			auto const n_bytes = std::min(count, self->triples.size());
			std::memcpy(buf, self->triples.data(), n_bytes);
			self->triples.remove_prefix(n_bytes);

			return n_bytes;
		}

		static int error([[maybe_unused]] void *voided_self) noexcept {
			return 0;
		}
	};

	SerdHandle serd_handle{.prefixes = {}, .add_triple_callback = std::move(add_triple_callback)};

	SerdReader *reader = serd_reader_new(SERD_NTRIPLES, (void *) &serd_handle,
										 nullptr,
										 reinterpret_cast<SerdBaseSink>(reject_base),
										 reinterpret_cast<SerdPrefixSink>(reject_prefix),
										 reinterpret_cast<SerdStatementSink>(on_statement_no_blank_nodes),
										 reinterpret_cast<SerdEndSink>(on_end));
	serd_reader_set_error_sink(reader, error_sink, nullptr);

	Source source{triples};
	serd_reader_read_source(reader, &Source::read, &Source::error, &source, nullptr, 4096);
	serd_reader_free(reader);
}
