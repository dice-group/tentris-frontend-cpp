# Tᴇɴᴛʀɪs-fontend

This provides frontends for the [Tᴇɴᴛʀɪs](https://github.com/dice-group/tentris-lib) library. Two binaries form the
frontend:

**tentris_loader** loads an RDF file into a disc-based index. **tentris_server** loads a specific index and provides
SPARQL endpoints to query the data stored in the loaded index.

## Production Build

You should use the [Dockerfile](./Dockerfile) to build tentris-frontend. It is based on alpine linux to provide fully
statically linked executables based on musl instead of glibc. Linking fully statically is broken with glibc.

Steps:

1. Make sure to provide credentials for https://conan.dice-research.org/artifactory/api/conan/tentris-private
    via the `CONAN_USER` and `CONAN_USER` build arguments.
2. Have an `ssh-agent` instance with your github ssh private key running. If it isn't already running, run:
    ```bash
    eval $(ssh-agent)
    ssh-add
    ```

3. Build the image using docker or podman:
    ```bash
    docker build . --tag tentris --build-arg CONAN_USER=xx --build-arg CONAN_PW=yyy --ssh=default
    ```

4. After you built the docker image you can extract the binary using:
    ```bash
    id=$(docker create tentris)
    docker cp $id:/tentris-server-rs .
    docker rm $id
    ```

## Development

Additionally required tools:
- clang >=14 or gcc >=12
- libstdc++(gcc) >=12
- CMake >=3.21
- conan >=1.59,<2.0

You must have [tentris](https://conan.dice-research.org/artifactory/api/conan/tentris)
and [tentris-private](https://conan.dice-research.org/artifactory/api/conan/tentris-private) in your conan remotes.
tentris-private requires credentials to be accessed.

```bash
cargo build # or cargo build --release
```
