import os
import re

import conan.tools.files
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout
from conans.tools import load


class TentrisConan(ConanFile):
    name = "tentris"
    version = None

    # Optional metadata
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of Tentris here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]
               }
    default_options = {"shared": False, "fPIC": True,
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
        "boost/1.78.0",
        "fmt/8.1.1",
        "restinio/0.6.14",
        "hypertrie/0.8.2@dice-group/stable",
        "sparql-parser-base/0.2.2@dice-group/stable",
        "dice-hash/0.3.0@dice-group/stable",
        "cxxopts/2.2.1",
        # override for conflict between sparql-parser and rdf-parser
        "robin-hood-hashing/3.11.5",
        "taskflow/3.3.0",
        "nlohmann_json/3.10.5",
        "spdlog/1.10.0",
        "vincentlaucsb-csv-parser/2.1.3",
    )

    generators = ("CMakeDeps", "CMakeToolchain")  # ("CMakeDeps", "cmake_find_package")

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "libs/*", "CMakeLists.txt", "cmake/*", "lib_conanfile.txt"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def set_version(self):
        if not hasattr(self, 'version') or self.version is None:
            cmake_file = load(os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.version = re.search(r"project\([^)]*VERSION\s+(\d+\.\d+.\d+)[^)]*\)", cmake_file).group(1)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()
