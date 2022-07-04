FROM alpine:3.16 AS builder
ARG DEBIAN_FRONTEND=noninteractive
ARG TENTRIS_MARCH="x86-64-v3"
ARG CONAN_USER="none"
ARG CONAN_PW="none"


RUN apk update && \
    apk add git make cmake boost-build pythonispython3 py3-pip autoconf automake gcc g++ clang \
    clang-dev clang-libs clang-extra-tools clang-static llvm13 llvm13-dev lld pkgconfig libuuid \
    libtool util-linux-dev linux-headers openjdk11-jdk && \
    apk add mold --repository=https://mirrors.edge.kernel.org/alpine/edge/testing

ARG CC="clang"
ARG CXX="clang++"
ENV CXXFLAGS="${CXXFLAGS} -march=${TENTRIS_MARCH}"

# Compile more recent tcmalloc-minimal with clang-14 + -march
RUN git clone --quiet --branch gperftools-2.9.1 --depth 1 https://github.com/gperftools/gperftools
WORKDIR /gperftools
RUN ./autogen.sh
RUN ./configure \
    --enable-minimal \
    --disable-debugalloc \
    --enable-sized-delete \
    --enable-dynamic-sized-delete-support && \
    make -j$(nproc) && \
    make install
WORKDIR /

# install and configure conan
RUN pip3 install conan && \
    conan user && \
    conan profile new --detect default && \
    conan profile update settings.compiler=clang default && \
    conan profile update settings.compiler.libcxx=libstdc++11 default && \
    conan profile update env.CXXFLAGS="${CXXFLAGS}" default && \
    conan profile update env.CXX="${CXX}" default && \
    conan profile update env.CC="${CC}" default && \
    conan profile update options.boost:extra_b2_flags="cxxflags=\\\"${CXXFLAGS}\\\"" default

# add conan repositories
RUN conan remote add dice-group https://conan.dice-research.org/artifactory/api/conan/tentris
RUN conan remote add tentris-private https://conan.dice-research.org/artifactory/api/conan/tentris-private
RUN conan user ${CONAN_USER} -p ${CONAN_PW} -r tentris-private

# build and cache dependencies via conan
WORKDIR /conan_cache
COPY lib_conanfile.txt conanfile.txt
RUN conan install . --build=missing --profile default

# import project files
WORKDIR /tentris
COPY thirdparty thirdparty
COPY libs libs
COPY execs execs
COPY cmake cmake
COPY CMakeLists.txt CMakeLists.txt
COPY conanfile.py conanfile.py
RUN sed -i 's/lld/mold/g' CMakeLists.txt

##build
WORKDIR /tentris/build
# todo: should be replaced with toolchain file like https://github.com/ruslo/polly/blob/master/clang-libcxx17-static.cmake
RUN cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DTENTRIS_BUILD_WITH_TCMALLOC=true \
    -DTENTRIS_STATIC=true \
    -DTENTRIS_MARCH=${TENTRIS_MARCH} \
    ..
RUN make -j $(nproc)
FROM scratch
COPY --from=builder /tentris/build/bin/tentris_server /tentris_server
COPY --from=builder /tentris/build/bin/tentris_loader /tentris_loader
COPY --from=builder /tentris/build/bin/deduplicated_nt /deduplicated_nt
COPY --from=builder /tentris/build/bin/rdf2ids /rdf2ids
COPY README.MD README.MD
ENTRYPOINT ["/tentris_server"]
