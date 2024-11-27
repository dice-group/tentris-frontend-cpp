cmake_minimum_required(VERSION 3.21)
add_subdirectory(rdf-tensor)
add_subdirectory(node-store)
add_subdirectory(sparql2tensor)
add_subdirectory(triple-store)
add_subdirectory(endpoint)
add_subdirectory(tentris)

if (BUILD_EINSUMSUM OR (BUILD_FFI AND WITH_HYPERTRIE_FFI))
    add_subdirectory(einsumsum)
endif ()

if (BUILD_FFI)
    add_subdirectory(ffi)
endif ()

include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_package()
