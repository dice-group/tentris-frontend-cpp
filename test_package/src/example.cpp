#include <Dice/node_store/PersistentNodeStorageBackend.hpp>

int main() {
	Dice::rdf_tensor::metall_manager x{metall::create_only, "abc"};
	Dice::node_store::PersistentNodeStorageBackendImpl y{x.get_allocator()};
}
