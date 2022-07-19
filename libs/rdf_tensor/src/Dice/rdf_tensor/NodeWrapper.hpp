#ifndef TENTRIS_NODEWRAPPER_HPP
#define TENTRIS_NODEWRAPPER_HPP

#include <Dice/hash/DiceHash.hpp>
#include <rdf4cpp/rdf.hpp>
//#include <Dice/rdf_tensor/check_no_throw.hpp>

namespace Dice::rdf_tensor {
	using namespace rdf4cpp::rdf;

	class NodeWrapper : public Node {
	protected:
		explicit NodeWrapper(NodeBackendHandle id) noexcept : Node(id) {}

	public:
		NodeWrapper() noexcept = default;

		NodeWrapper(Node node) noexcept : Node(node) {}

		bool operator==(const NodeWrapper &other) const noexcept {
			return this->backend_handle().raw() == other.backend_handle().raw();
		}

		bool operator!=(const NodeWrapper &other) const noexcept {
			return this->backend_handle().raw() != other.backend_handle().raw();
		}

		auto operator<=>(const NodeWrapper &other) const noexcept {
			return this->backend_handle().raw() <=> other.backend_handle().raw();
		};

		operator std::optional<Node>() const noexcept {
			return (Node) * this;
		};

		operator bool() const noexcept {
			assert(handle_.is_literal());
			auto literal_backend = handle_.literal_backend();
			if (literal_backend.lexical_form == "0")
				return false;
			return true;
		}

	};
};// namespace Dice::rdf_tensor

template<typename Policy>
struct Dice::hash::dice_hash_overload<Policy, Dice::rdf_tensor::NodeWrapper> {
	inline static std::size_t dice_hash(Dice::rdf_tensor::NodeWrapper const &x) noexcept {
		return Policy::hash_fundamental(x.backend_handle().raw());
	}
};

template<>
struct std::hash<Dice::rdf_tensor::NodeWrapper> {
	size_t operator()(Dice::rdf_tensor::NodeWrapper const &x) const noexcept {
		return x.backend_handle().raw();
	}
};

#endif//TENTRIS_NODEWRAPPER_HPP
