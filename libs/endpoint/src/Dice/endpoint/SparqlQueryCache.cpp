#include "SparqlQueryCache.hpp"

#include <Dice/sparql2tensor/SPARQLQuery.hpp>

namespace Dice::endpoint {
	template class SyncedLRUCache<std::string, Dice::sparql2tensor::SPARQLQuery>;
}// namespace Dice::endpoint