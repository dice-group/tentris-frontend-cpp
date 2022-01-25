#ifndef TENTRIS_TSLSPARSEMAPNODESTORAGEBACKEND_HPP
#define TENTRIS_TSLSPARSEMAPNODESTORAGEBACKEND_HPP

#include "Dice/node_store/TslSparseMapNodeStorageBackendImpl.hpp"

namespace Dice::node_storage {

	class TslSparseMapNodeStorageBackend : public rdf4cpp::rdf::storage::node::INodeStorageBackend {
		TslSparseMapNodeStorageBackendImpl *impl_;

	public:
		explicit TslSparseMapNodeStorageBackend(TslSparseMapNodeStorageBackendImpl *impl);

		~TslSparseMapNodeStorageBackend() override = default;

		rdf4cpp::rdf::storage::node::identifier::NodeID get_string_literal_id(std::string_view lexical_form) override;
		rdf4cpp::rdf::storage::node::identifier::NodeID get_typed_literal_id(std::string_view lexical_form, std::string_view datatype) override;
		rdf4cpp::rdf::storage::node::identifier::NodeID get_typed_literal_id(std::string_view lexical_form, const rdf4cpp::rdf::storage::node::identifier::NodeID &datatype_id) override;
		rdf4cpp::rdf::storage::node::identifier::NodeID get_lang_literal_id(std::string_view lexical_form, std::string_view lang) override;
		rdf4cpp::rdf::storage::node::identifier::NodeID get_iri_id(std::string_view iri) override;
		rdf4cpp::rdf::storage::node::identifier::NodeID get_variable_id(std::string_view identifier, bool anonymous) override;
		rdf4cpp::rdf::storage::node::identifier::NodeID get_bnode_id(std::string_view identifier) override;
		[[nodiscard]] rdf4cpp::rdf::storage::node::handle::IRIBackendView get_iri_handle(rdf4cpp::rdf::storage::node::identifier::NodeIDValue id) const override;
		[[nodiscard]] rdf4cpp::rdf::storage::node::handle::LiteralBackendView get_literal_handle(rdf4cpp::rdf::storage::node::identifier::NodeIDValue id) const override;
		[[nodiscard]] rdf4cpp::rdf::storage::node::handle::BNodeBackendView get_bnode_handle(rdf4cpp::rdf::storage::node::identifier::NodeIDValue id) const override;
		[[nodiscard]] rdf4cpp::rdf::storage::node::handle::VariableBackendView get_variable_handle(rdf4cpp::rdf::storage::node::identifier::NodeIDValue id) const override;
	};
}// namespace Dice::node_storage

#endif//TENTRIS_TSLSPARSEMAPNODESTORAGEBACKEND_HPP
