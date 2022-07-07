FROM alpine:edge AS builder
# todo: fix version as soon as clang-14 is available outside of edge
ARG MARCH="x86-64-v3"
ARG CONAN_USER="none"
ARG CONAN_PW="none"


RUN apk update && \
    apk add git make cmake boost-build pythonispython3 py3-pip autoconf automake gcc g++ clang \
    clang-dev clang-libs clang-extra-tools clang-static llvm14 llvm14-dev lld pkgconfig libuuid \
    libtool util-linux-dev linux-headers openjdk11-jdk && \
    apk add mold --repository=https://mirrors.edge.kernel.org/alpine/edge/testing

ARG CC="clang"
ARG CXX="clang++"
ENV CXXFLAGS="${CXXFLAGS} -march=${MARCH}"
RUN rm /usr/bin/ld && ln -s /usr/bin/mold /usr/bin/ld # use mold as default linker


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
COPY conanfile.py .
COPY CMakeLists.txt .
RUN conan install . --build=missing --profile default
# import project files
WORKDIR /tentris
COPY libs libs
COPY execs execs
COPY cmake cmake
COPY CMakeLists.txt .
COPY conanfile.py .

##build
WORKDIR /tentris/execs/build
# todo: should be replaced with toolchain file like https://github.com/ruslo/polly/blob/master/clang-libcxx17-static.cmake
RUN cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DWITH_TCMALLOC=true \
    -DSTATIC=true \
    -DMARCH=${MARCH} \
    ..
RUN make -j $(nproc)
RUN ls -lah tools/deduplicated_nt

FROM scratch
COPY --from=builder /tentris/execs/build/tentris_server/tentris_server /tentris_server
COPY --from=builder /tentris/execs/build/tentris_loader/tentris_loader /tentris_loader
COPY --from=builder /tentris/execs/build/tools/deduplicated_nt/deduplicated_nt /deduplicated_nt
COPY --from=builder /tentris/execs/build/tools/rdf2ids/rdf2ids /rdf2ids
COPY README.MD README.MD
ENTRYPOINT ["/tentris_server"]
