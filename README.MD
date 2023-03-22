# Tᴇɴᴛʀɪs-fontend

This provides frontends for the [Tᴇɴᴛʀɪs](https://github.com/dice-group/tentris-lib) library. Two binaries form the
frontend:

**tentris_loader** loads an RDF file into a disc-based index. **tentris_server** loads a specific index and provides
SPARQL endpoints to query the data stored in the loaded index.

# Production Build

You should use the [Dockerfile](./Dockerfile) to build tentris-frontend. It is based on alpine linux to provide fully
statically linked executables based on musl instead of glibc. Linking fully statically is broken with glibc.

Make sure to provide credentials
for https://conan.dice-research.org/artifactory/api/conan/tentris-private via `CONAN_USER` and `CONAN_USER`:

```shell script
docker build -f Dockerfile --build-arg CONAN_USER=xx --build-arg CONAN_PW=yyy .
```

After you built the docker image you can extract the binaries with:

```shell script
id=$(docker create tentris_docker_alpine:latest)
docker export $id | tar -x --wildcards "tentris_*"
docker rm $id
```

# Development

Required tools: clang >=14 or gcc >=12, libstdc++(gcc) >=12, CMake >=3.21, conan >=1.59,<2.0

You must have [tentris](https://conan.dice-research.org/artifactory/api/conan/tentris)
and [tentris-private](https://conan.dice-research.org/artifactory/api/conan/tentris-private) in your conan remotes.
tentris-private requires credentials to be accessed.

There are two CMake files of relevance:

- The library tentris-frontend providesare managed at [CMakeLists.txt](./CMakeLists.txt)
  ```bash
  cd execs
  cmake --build build_dir --config Release # or Debug/RelWithDebInfo/MinSizeRel
  cmake -G Ninja -B build_dir
  ```

  Additionally, these CMake options are available:

    - `-DBUILD_SHARED_LIBS=ON/OFF [default: OFF]`: Build a shared library instead of a static one.
    - `-DUSE_CONAN=ON/OFF [default: ON]`: If available and this is the top project, use Conan to retrieve dependencies.

- The binaries are built via [execs/CMakeLists.txt](./execs/CMakeLists.txt) that includes
  ```bash
  cmake --build build_dir --config Release # or Debug/RelWithDebInfo/MinSizeRel
  cmake -G Ninja -B build_dir
  ```
  Additionally, these CMake options are available:

    - `-DSTATIC=ON/OFF [default: OFF]`: Link binaries fully statically.
    - `-DWITH_TCMALLOC=ON/OFF [default: OFF]`: Link and use tcmalloc.

  