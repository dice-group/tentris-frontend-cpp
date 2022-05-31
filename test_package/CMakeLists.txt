cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

find_package(triple_store REQUIRED)
find_package(node_store REQUIRED)

add_executable(example src/example.cpp)
target_link_libraries(example Dice::triple_store Dice::node_store)
