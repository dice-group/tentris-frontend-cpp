import os
import re

from conans import ConanFile, CMake
from conans.tools import load
from conans.util.files import rmdir


class Recipe(ConanFile):
    url = "https://tentris.dice-research.org"
    topics = ("triplestore", "sparql", "rdf", "sematic-web", "tensor")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_exec_deps": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_exec_deps": False,
        "boost:header_only": True,  # override hypertrie settings # TODO: remove in hypertrie and here
        "restinio:asio": "boost",
    }

    def requirements(self):
        public_reqs = [
            "boost/1.79.0",
            "fmt/8.1.1",
            "restinio/0.6.15",
            "expected-lite/0.6.0",  # overrides restinio dependency
            "hypertrie/0.9.0@dice-group/rc1",
            "metall/0.20@dice-group/stable",
            "rdf4cpp/0.0.4",
            "dice-hash/0.3.0@dice-group/stable",
            "robin-hood-hashing/3.11.5",
            "cxxopts/2.2.1",
            "serd/0.30.13-f6437f",
            "sparql-parser-base/0.2.2@dice-group/stable",
            "taskflow/3.3.0",
            "cppitertools/2.1",
            "spdlog/1.10.0",
            "rapidjson/cci.20211112",
        ]

        private_reqs = [
        ]

        exec_reqs = [
            "nlohmann_json/3.10.5",
            "vincentlaucsb-csv-parser/2.1.3",
        ]
        for req in public_reqs:
            self.requires(req)
        for req in private_reqs:
            self.requires(req, private=True)

        if self.options.get_safe("with_exec_deps"):
            for req in exec_reqs:
                self.requires(req)

    generators = ("cmake_find_package",)

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "libs/*", "CMakeLists.txt", "cmake/*"

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

    _cmake = None

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions['CONAN_CMAKE'] = False
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
        for dir in ("res", "share"):
            rmdir(os.path.join(self.package_folder, dir))

    def package_info(self):
        self.cpp_info.components["global"].set_property("cmake_target_name", "tentris::tentris")
        self.cpp_info.components["global"].names["cmake_find_package_multi"] = "tentris"
        self.cpp_info.components["global"].names["cmake_find_package"] = "tentris"
        self.cpp_info.components["global"].includedirs = [f"include/tentris/tentris"]
        self.cpp_info.components["global"].libdirs = []
        self.cpp_info.set_property("cmake_file_name", "tentris")
        self.cpp_info.components["global"].requires = [
            "node_store", "rdf_tensor", "sparql2tensor", "triple_store",
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
            "expected-lite::expected-lite",
            "restinio::restinio",
            "taskflow::taskflow",
            "cppitertools::cppitertools",
            "spdlog::spdlog",
        ]

        for component in ["node_store", "rdf_tensor", "sparql2tensor", "triple_store", "endpoint"]:
            self.cpp_info.components[f"{component}"].names["cmake_find_package_multi"] = f"{component}"
            self.cpp_info.components[f"{component}"].names["cmake_find_package"] = f"{component}"
            self.cpp_info.components[f"{component}"].includedirs = [f"include/tentris/{component}"]

        for component in ["node_store", "sparql2tensor", "triple_store", "endpoint"]:
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
        self.cpp_info.components["endpoint"].requires = [
            "rdf_tensor",
            "restinio::restinio",
            "taskflow::taskflow",
            "cppitertools::cppitertools",
            "spdlog::spdlog",
            "rapidjson::rapidjson",
        ]
        if self.options.get_safe("with_exec_deps"):
            self.cpp_info.components["global"].requires += [
                "vincentlaucsb-csv-parser",
                "nlohmann_json::nlohmann_json"]
