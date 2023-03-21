import os
import re

from conan.tools.cmake import CMake
from conan.tools.files import load, rmdir
from conan import ConanFile


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
        "restinio:asio": "boost",
    }

    def requirements(self):
        public_reqs = [
            "tentris/2.1.0",
            "boost/1.80.0",
            "fmt/8.1.1",
            "restinio/0.6.17",
            "expected-lite/0.6.2",  # overrides restinio dependency
            "robin-hood-hashing/3.11.5",
            "cxxopts/2.2.1",
            "taskflow/3.4.0",
            "cppitertools/2.1",
            "spdlog/1.10.0",
            "rapidjson/cci.20220822",
            "pugixml/1.13",
        ]

        private_reqs = [
        ]

        exec_reqs = [
            "nlohmann_json/3.11.2",
            "vincentlaucsb-csv-parser/2.1.3",
        ]
        for req in public_reqs:
            self.requires(req)
        for req in private_reqs:
            self.requires(req, private=True)

        if self.options.get_safe("with_exec_deps"):
            for req in exec_reqs:
                self.requires(req)

    generators = ("CMakeDeps", "CMakeToolchain")

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "libs/*", "CMakeLists.txt", "cmake/*"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def set_name(self):
        if not hasattr(self, 'name') or self.version is None:
            cmake_file = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.name = re.search(r"project\(\s*([a-z\-]+)\s+VERSION", cmake_file).group(1)

    def set_version(self):
        if not hasattr(self, 'version') or self.version is None:
            cmake_file = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.version = re.search(r"project\([^)]*VERSION\s+(\d+\.\d+.\d+)[^)]*\)", cmake_file).group(1)
        if not hasattr(self, 'description') or self.description is None:
            cmake_file = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.description = re.search(r"project\([^)]*DESCRIPTION\s+\"([^\"]+)\"[^)]*\)", cmake_file).group(1)

    _cmake = None

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.configure(variables={"USE_CONAN": False})
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        self._configure_cmake().install()
        rmdir(self, os.path.join(self.package_folder, "cmake"))
        rmdir(self, os.path.join(self.package_folder, "share"))

    def package_info(self):
        self.cpp_info.set_property("cmake_target_name", f"{self.name}")
        self.cpp_info.set_property("cmake_file_name", f"{self.name}")

        self.cpp_info.components["global"].set_property("cmake_target_name", f"{self.name}::{self.name}")
        self.cpp_info.components["global"].names["cmake_find_package_multi"] = f"{self.name}"
        self.cpp_info.components["global"].names["cmake_find_package"] = f"{self.name}"
        self.cpp_info.components["global"].includedirs = [f"include/{self.name}/{self.name}"]
        self.cpp_info.components["global"].libdirs = []
        self.cpp_info.components["global"].includedirs = [f"include/{self.name}/{self.name}"]
        self.cpp_info.components["global"].requires = [
            "endpoint",
            "boost::boost",
            "fmt::fmt",
            "restinio::restinio",
            "cxxopts::cxxopts",
            "robin-hood-hashing::robin-hood-hashing",
            "expected-lite::expected-lite",
            "restinio::restinio",
            "taskflow::taskflow",
            "cppitertools::cppitertools",
        ]

        for component in ["endpoint"]:
            self.cpp_info.components[f"{component}"].names["cmake_find_package_multi"] = f"{component}"
            self.cpp_info.components[f"{component}"].names["cmake_find_package"] = f"{component}"
            self.cpp_info.components[f"{component}"].includedirs = [f"include/{self.name}/{component}"]

        self.cpp_info.components["endpoint"].requires = [
            "tentris::tentris",
            "restinio::restinio",
            "taskflow::taskflow",
            "cppitertools::cppitertools",
            "spdlog::spdlog",
            "rapidjson::rapidjson",
            "pugixml::pugixml"
        ]
        if self.options.get_safe("with_exec_deps"):
            self.cpp_info.components["global"].requires += [
                "vincentlaucsb-csv-parser::vincentlaucsb-csv-parser",
                "nlohmann_json::nlohmann_json"]
