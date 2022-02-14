#ifndef TENTRIS_SERDLOAD_HPP
#define TENTRIS_SERDLOAD_HPP

#include <rdf4cpp/rdf.hpp>


namespace Dice::triple_store {

	using AddTripleCallback_function = std::function<void(rdf4cpp::rdf::Node, rdf4cpp::rdf::Node, rdf4cpp::rdf::Node)>;

	void serd_load(std::string const &file_path, AddTripleCallback_function add_triple_callback);

}// namespace Dice::triple_store
#endif//TENTRIS_SERDLOAD_HPP
