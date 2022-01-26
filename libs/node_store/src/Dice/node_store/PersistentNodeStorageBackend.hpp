#ifndef TENTRIS_PERSISTENTNODESTORAGEBACKEND_HPP
#define TENTRIS_PERSISTENTNODESTORAGEBACKEND_HPP

#include "Dice/node_store/PersistentNodeStorageBackendImpl.hpp"

namespace Dice::node_store {

	class PersistentNodeStorageBackend : public rdf4cpp::rdf::storage::node::INodeStorageBackend {
		PersistentNodeStorageBackendImpl *impl_;

	public:
		explicit PersistentNodeStorageBackend(PersistentNodeStorageBackendImpl *impl);

		~PersistentNodeStorageBackend() override = default;

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
}// namespace Dice::node_store

#endif//TENTRIS_PERSISTENTNODESTORAGEBACKEND_HPP
