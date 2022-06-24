import os
import re

from conans import ConanFile, CMake
from conans.tools import load
from conans.util.files import rmdir


class Recipe(ConanFile):

    # Optional metadata
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of Tentris here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_test_deps": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_test_deps": False,
        "restinio:asio": "boost",
        "restinio:with_zlib": True,
        "boost:header_only": False,  # override hypertrie settings
        "boost:without_context": True,
        "boost:without_contract": True,
        "boost:without_coroutine": True,
        "boost:without_fiber": True,
        "boost:without_graph": True,
        "boost:without_graph_parallel": True,
        "boost:without_iostreams": True,
        "boost:without_json": True,
        "boost:without_locale": True,
        "boost:without_math": True,
        "boost:without_mpi": True,
        "boost:without_nowide": True,
        "boost:without_program_options": True,
        "boost:without_python": True,
        "boost:without_serialization": True,
        "boost:without_stacktrace": True,
        "boost:without_test": True,
        "boost:without_timer": True,
        "boost:without_type_erasure": True,
        "boost:without_wave": True}
    requires = (
        "boost/1.79.0",
        "fmt/8.1.1",
        "restinio/0.6.15",
        "hypertrie/0.9.0@dice-group/rc1",
        "metall/0.20@dice-group/stable",
        "serd/0.30.13-f6437f", # private dependency
        "rdf4cpp/0.0.4@dice-group/experimental",
        "sparql-parser-base/0.2.2@dice-group/stable",
        "dice-hash/0.3.0@dice-group/stable",
        "cxxopts/2.2.1",
        # override for conflict between sparql-parser and rdf-parser
        "robin-hood-hashing/3.11.5",
        # "taskflow/3.3.0",
        # "cppitertools/2.1",
        # "rapidjson/cci.20211112",
        # "nlohmann_json/3.10.5",
        # "spdlog/1.10.0",
        # "vincentlaucsb-csv-parser/2.1.3",
    )

    generators = ("cmake_find_package",) # ("cmake_find_package", "CMakeDeps", "CMakeToolchain")  # ("CMakeDeps", "cmake_find_package")

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "libs/*", "CMakeLists.txt", "cmake/*", "lib_conanfile.txt"

    def build_requirements(self):
        # useful for example for conditional build_requires
        pass # todo: use for e.g. sparql-parser-base?

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def set_name(self):
        if not hasattr(self, 'name') or self.version is None:
            cmake_file = load(os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.name = re.search(r"project\(\s*([a-z\-]+)\s+VERSION", cmake_file).group(1)

    def set_version(self):
        if not hasattr(self, 'version') or self.version is None:
            cmake_file = load(os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.version = re.search(r"project\([^)]*VERSION\s+(\d+\.\d+.\d+)[^)]*\)", cmake_file).group(1)
        if not hasattr(self, 'description') or self.description is None:
            cmake_file = load(os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.description = re.search(r"project\([^)]*DESCRIPTION\s+\"([^\"]+)\"[^)]*\)", cmake_file).group(1)

    def build(self):
        cmake = CMake(self)
        cmake.definitions['CONAN_CMAKE'] = False
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        for dir in ("res", "share"):
            rmdir(os.path.join(self.package_folder, dir))

    def package_info(self):  #
        # self.cpp_info.set_property("cmake_target_name", "tentris")
        self.cpp_info.components["global"].set_property("cmake_target_name", "tentris::tentris")
        self.cpp_info.components["global"].names["cmake_find_package_multi"] = "tentris"
        self.cpp_info.components["global"].names["cmake_find_package"] = "tentris"
        self.cpp_info.set_property("cmake_file_name", "tentris")
        self.cpp_info.components["global"].includedirs = ["include/tentris/tentris/"]
        self.cpp_info.components["global"].requires = [
            "boost::boost",
            "fmt::fmt",
            "restinio::restinio",
            "hypertrie::hypertrie",
            "metall::metall",
            "rdf4cpp::rdf4cpp",
            "sparql-parser-base::sparql-parser-base",
            "dice-hash::dice-hash",
            # "dice-sparse-map::dice-sparse-map",
            "cxxopts::cxxopts",
            "robin-hood-hashing::robin-hood-hashing",
        ]
        # "robin-hood-hashing/3.11.5",
        # "taskflow/3.3.0",
        # "nlohmann_json/3.10.5",
        # "spdlog/1.10.0",
        # "vincentlaucsb-csv-parser/2.1.3",
        if self.options.with_test_deps:
            pass

        for component in ["node_store", "rdf_tensor", "sparql2tensor", "triple_store"]: # "endpoint"
            self.cpp_info.components[f"{component}"].names["cmake_find_package_multi"] = f"{component}"
            self.cpp_info.components[f"{component}"].names["cmake_find_package"] = f"{component}"
            self.cpp_info.components[f"{component}"].includedirs = [f"include/tentris/{component}"]

        for component in ["node_store", "sparql2tensor", "triple_store"]: # "endpoint"
            self.cpp_info.components[f"{component}"].libdirs = [f"lib/tentris/{component}"]
            self.cpp_info.components[f"{component}"].libs = [f"{component}"]


        self.cpp_info.components["rdf_tensor"].requires = [
            "rdf4cpp::rdf4cpp",
            "hypertrie::hypertrie",
            "boost::boost",
            "metall::metall",
        ]

        self.cpp_info.components["node_store"].requires = [
            "rdf_tensor",
        ]

        self.cpp_info.components["sparql2tensor"].requires = [
            "node_store",
            "robin-hood-hashing::robin-hood-hashing",
            "sparql-parser-base::sparql-parser-base",
        ]

        self.cpp_info.components["triple_store"].requires = [
            "sparql2tensor",
            "rdf_tensor",
            "serd::serd"
        ]

        # self.cpp_info.components["endpoint"].requires = [
        #     "rdf_tensor",
        #     "restinio::restinio",
        #     "taskflow::taskflow",
        #     "cppitertools::cppitertools", # private
        #     "spdlog::spdlog", # public
        #     "rapidjson::rapidjson", #private
        # ]