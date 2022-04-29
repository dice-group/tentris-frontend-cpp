#ifndef TENTRIS_NODEWRAPPER_HPP
#define TENTRIS_NODEWRAPPER_HPP

#include <Dice/hash/DiceHash.hpp>
#include <rdf4cpp/rdf.hpp>

namespace Dice::rdf_tensor {
	using namespace rdf4cpp::rdf;

	class NodeWrapper : public Node {
	protected:
		explicit NodeWrapper(NodeBackendHandle id) noexcept : Node(id) {}

	public:
		NodeWrapper() noexcept = default;

		NodeWrapper(Node node) noexcept : Node(node) {}

		bool operator==(const NodeWrapper &other) const noexcept {
			return this->backend_handle() == other.backend_handle();
		}

		auto operator<=>(const NodeWrapper &other) const noexcept {
			return this->backend_handle() <=> other.backend_handle();
		};

		operator std::optional<Node>() const noexcept {
			return (Node) * this;
		};
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
	inline size_t operator()(Dice::rdf_tensor::NodeWrapper const &v) const noexcept {
		return Dice::hash::DiceHash<Dice::rdf_tensor::NodeWrapper, Dice::hash::Policies::Martinus>()(v);
	}
};

#endif//TENTRIS_NODEWRAPPER_HPP
