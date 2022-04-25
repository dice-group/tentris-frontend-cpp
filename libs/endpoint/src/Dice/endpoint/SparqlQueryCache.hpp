#ifndef TENTRIS_SPARQLQUERYCACHE_HPP
#define TENTRIS_SPARQLQUERYCACHE_HPP

#include <Dice/sparql2tensor/SPARQLQuery.hpp>

#include <Dice/endpoint/SyncedLRUCache.hpp>

namespace Dice::endpoint {

	using SparqlQueryCache = SyncedLRUCache<std::string, sparql2tensor::SPARQLQuery>;

}
#endif//TENTRIS_SPARQLQUERYCACHE_HPP
