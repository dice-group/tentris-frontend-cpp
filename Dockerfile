FROM ubuntu:22.04 AS builder
ARG DEBIAN_FRONTEND=noninteractive
ARG TENTRIS_MARCH="x86-64-v3"
ARG CONAN_USER="none"
ARG CONAN_PW="none"


RUN apt-get -qq update && \
    apt-get -qq install -y make cmake uuid-dev git openjdk-11-jdk-headless python-is-python3 python3-pip python3-setuptools python3-wheel libstdc++-11-dev clang-14 g++-11 pkg-config lld-14 autoconf libtool
RUN rm /usr/bin/ld && ln -s /usr/bin/lld-14 /usr/bin/ld
ARG CXX="clang++-14"
ARG CC="clang-14"
ENV CXXFLAGS="${CXXFLAGS} -march=${TENTRIS_MARCH}"
ENV CMAKE_EXE_LINKER_FLAGS="-L/usr/local/lib/x86_64-linux-gnu -L/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -L/usr/local/lib"

# Compile more recent tcmalloc-minimal with clang-14 + -march
RUN git clone --quiet --branch gperftools-2.9.1 --depth 1 https://github.com/gperftools/gperftools
WORKDIR /gperftools
RUN ./autogen.sh
RUN export LDFLAGS="${CMAKE_EXE_LINKER_FLAGS}" && ./configure \
    --enable-minimal \
    --disable-debugalloc \
    --enable-sized-delete \
    --enable-dynamic-sized-delete-support && \
    make -j $(nproc) && \
    make install
WORKDIR /

# we need serd as static library. Not available from ubuntu repos
RUN git clone --quiet --branch fix-stack-leak --depth 1 --recurse-submodules --shallow-submodules https://gitlab.com/drobilla/serd.git
WORKDIR /serd
RUN ./waf configure --static && \
    ./waf install
WORKDIR /

# install and configure conan
RUN pip3 install conan && \
    conan user && \
    conan profile new --detect default && \
    conan profile update settings.compiler.libcxx=libstdc++11 default && \
    conan profile update env.CXXFLAGS="${CXXFLAGS}" default && \
    conan profile update env.CMAKE_EXE_LINKER_FLAGS="${CMAKE_EXE_LINKER_FLAGS}" default && \
    conan profile update env.CXX="${CXX}" default && \
    conan profile update env.CC="${CC}" default && \
    conan profile update options.boost:extra_b2_flags="cxxflags=\\\"${CXXFLAGS}\\\"" default

# add conan repositories
RUN conan remote add dice-group https://conan.dice-research.org/artifactory/api/conan/tentris
RUN conan remote add tentris-private https://conan.dice-research.org/artifactory/api/conan/tentris-private
RUN conan user ${CONAN_USER} -p ${CONAN_PW} -r tentris-private

# build and cache dependencies via conan
WORKDIR /conan_cache
COPY conanfile.txt conanfile.txt
RUN conan install . --build=missing --profile default

# import project files
WORKDIR /tentris
COPY thirdparty thirdparty
COPY libs libs
COPY execs execs
COPY cmake cmake
COPY CMakeLists.txt CMakeLists.txt
COPY conanfile.txt conanfile.txt

##build
WORKDIR /tentris/build
# todo: should be replaced with toolchain file like https://github.com/ruslo/polly/blob/master/clang-libcxx17-static.cmake
RUN cmake \
    -DCMAKE_EXE_LINKER_FLAGS="${CMAKE_EXE_LINKER_FLAGS}" \
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
