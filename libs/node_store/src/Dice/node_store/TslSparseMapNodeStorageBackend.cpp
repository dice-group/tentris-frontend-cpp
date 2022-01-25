#include "TslSparseMapNodeStorageBackend.hpp"
namespace Dice::node_storage {

	TslSparseMapNodeStorageBackend::TslSparseMapNodeStorageBackend(TslSparseMapNodeStorageBackendImpl *impl)
		: INodeStorageBackend(), impl_(impl) {}
	rdf4cpp::rdf::storage::node::identifier::NodeID TslSparseMapNodeStorageBackend::get_string_literal_id(std::string_view lexical_form) {
		return impl_->get_string_literal_id(lexical_form);
	}
	rdf4cpp::rdf::storage::node::identifier::NodeID TslSparseMapNodeStorageBackend::get_typed_literal_id(std::string_view lexical_form, std::string_view datatype) {
		return impl_->get_typed_literal_id(lexical_form, datatype);
	}
	rdf4cpp::rdf::storage::node::identifier::NodeID TslSparseMapNodeStorageBackend::get_typed_literal_id(std::string_view lexical_form, const rdf4cpp::rdf::storage::node::identifier::NodeID &datatype_id) {
		return impl_->get_typed_literal_id(lexical_form, datatype_id);
	}
	rdf4cpp::rdf::storage::node::identifier::NodeID TslSparseMapNodeStorageBackend::get_lang_literal_id(std::string_view lexical_form, std::string_view lang) {
		return impl_->get_lang_literal_id(lexical_form, lang);
	}
	rdf4cpp::rdf::storage::node::identifier::NodeID TslSparseMapNodeStorageBackend::get_iri_id(std::string_view iri) {
		return impl_->get_iri_id(iri);
	}
	rdf4cpp::rdf::storage::node::identifier::NodeID TslSparseMapNodeStorageBackend::get_variable_id(std::string_view identifier, bool anonymous) {
		return impl_->get_variable_id(identifier, anonymous);
	}
	rdf4cpp::rdf::storage::node::identifier::NodeID TslSparseMapNodeStorageBackend::get_bnode_id(std::string_view identifier) {
		return impl_->get_bnode_id(identifier);
	}
	rdf4cpp::rdf::storage::node::handle::IRIBackendView TslSparseMapNodeStorageBackend::get_iri_handle(rdf4cpp::rdf::storage::node::identifier::NodeIDValue id) const {
		return impl_->get_iri_handle(id);
	}
	rdf4cpp::rdf::storage::node::handle::LiteralBackendView TslSparseMapNodeStorageBackend::get_literal_handle(rdf4cpp::rdf::storage::node::identifier::NodeIDValue id) const {
		return impl_->get_literal_handle(id);
	}
	rdf4cpp::rdf::storage::node::handle::BNodeBackendView TslSparseMapNodeStorageBackend::get_bnode_handle(rdf4cpp::rdf::storage::node::identifier::NodeIDValue id) const {
		return impl_->get_bnode_handle(id);
	}
	rdf4cpp::rdf::storage::node::handle::VariableBackendView TslSparseMapNodeStorageBackend::get_variable_handle(rdf4cpp::rdf::storage::node::identifier::NodeIDValue id) const {
		return impl_->get_variable_handle(id);
	}
}// namespace Dice::node_storage