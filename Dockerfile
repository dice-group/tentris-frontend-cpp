FROM rust:1.72.0-alpine3.17 AS builder

RUN apk update && \
    apk add \
    make cmake autoconf automake pkgconfig \
    gcc g++ gdb \
    clang15 clang15-static clang15-dev clang15-libs clang15-static llvm15 llvm15-static llvm15-dev \
    compiler-rt musl-dev \
    openjdk11-jdk \
    pythonispython3 py3-pip \
    bash git libtool util-linux-dev linux-headers curl ninja \
    && \
    apk add mold --repository=https://mirrors.edge.kernel.org/alpine/edge/testing

# Install nightly
RUN rustup toolchain install nightly-x86_64-unknown-linux-musl

# Copy over required compiler wrappers for alpine
COPY --chmod=755 docker/clangxx.wrap /usr/local/bin
COPY --chmod=755 docker/rustc.wrap /usr/local/bin

# Copy over cargo config
RUN mkdir -p ~/.cargo
COPY docker/config.toml ~/.cargo/config.toml

# Ensure only mold is used to link
# And ensure linker finds static C runtime
RUN rm -f /usr/bin/ld && \
    ln -s /usr/bin/mold /usr/bin/ld && \
    ln -s /usr/lib/gcc/x86_64-alpine-linux-musl/12.2.1/* /usr/lib/

ARG MARCH="x86-64-v3"
ARG MTUNE=""
ENV CC="/usr/bin/clang"
ENV CFLAGS="${CFLAGS} -march=${MARCH} -mtune=${MTUNE}"
ENV CXX="/usr/local/bin/clangxx.wrap"
ENV CXXFLAGS="${CXXFLAGS} -march=${MARCH} -mtune=${MTUNE}"
ENV RUSTC="/usr/local/bin/rustc.wrap"
ENV RUSTFLAGS="${RUSTFLAGS} -C target-cpu=${MARCH} -Z tune-cpu=${MTUNE}"

ARG CONAN_USER="none"
ARG CONAN_PW="none"

# Install and configure conan
# TODO PyYAML~6 doesn't really want to build right now so explicitly choosing 5.3
RUN pip3 install PyYAML==5.3 conan==1.60.1 && \
    conan user && \
    conan profile new --detect default && \
    conan profile update settings.compiler=clang default && \
    conan profile update settings.compiler.version=15 default && \
    conan profile update settings.compiler.libcxx=libstdc++11 default && \
    conan profile update settings.compiler.cppstd=20 default && \
    conan profile update env.CXXFLAGS="${CXXFLAGS}" default && \
    conan profile update env.CXX="${CXX}" default && \
    conan profile update env.CC="${CC}" default

# Add conan repositories
RUN conan remote add dice-group https://conan.dice-research.org/artifactory/api/conan/tentris && \
    conan remote add tentris-private https://conan.dice-research.org/artifactory/api/conan/tentris-private && \
    conan user ${CONAN_USER} -p ${CONAN_PW} -r tentris-private

# Import project files
WORKDIR /usr/local/src/tentris-frontend
COPY Cargo.toml Cargo.toml
COPY Cargo.lock Cargo.lock
COPY src src

# Using ssh in container because it doesn't leak access tokens.
# Unfortunately conan can't really do that.
RUN sed -i 's|https://github.com/|ssh://git@github.com/|g' Cargo.toml

# SSH key needs to be able to access to https://github.com/dice-group/tentris-lib-rs
RUN --mount=type=ssh cargo +nightly build -vv --release --features static-build
RUN ldd target/release/tentris

FROM scratch
COPY --from=builder /usr/local/src/tentris-frontend/target/release/tentris /tentris
ENTRYPOINT ["/tentris", "-s", "/data", "serve"]
