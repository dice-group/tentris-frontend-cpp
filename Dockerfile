FROM ubuntu:impish AS builder
ARG DEBIAN_FRONTEND=noninteractive
ARG TENTRIS_MARCH="x86-64"
ARG CONAN_USER="none"
ARG CONAN_PW="none"


RUN apt-get -qq update && \
    apt-get -qq install -y make cmake uuid-dev git openjdk-11-jdk python3-pip python3-setuptools python3-wheel libstdc++-11-dev clang-13 g++-11 pkg-config lld-13 autoconf libtool
RUN rm /usr/bin/ld && ln -s /usr/bin/lld-13 /usr/bin/ld
ARG CXX="clang++-13"
ARG CC="clang-13"
ENV CXXFLAGS="${CXXFLAGS} -march=${TENTRIS_MARCH}"
ENV CMAKE_EXE_LINKER_FLAGS="-L/usr/local/lib/x86_64-linux-gnu -L/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -L/usr/local/lib"

# Compile more recent tcmalloc-minimal with clang-13 + -march
RUN git clone --quiet --branch gperftools-2.9.1 https://github.com/gperftools/gperftools
WORKDIR /gperftools
RUN ./autogen.sh
RUN export LDFLAGS="${CMAKE_EXE_LINKER_FLAGS}" && ./configure \
    --enable-minimal \
    --disable-debugalloc \
    --enable-sized-delete \
    --enable-dynamic-sized-delete-support && \
    make -j && \
    make install
WORKDIR /

# we need serd as static library. Not available from ubuntu repos
RUN ln -s /usr/bin/python3 /usr/bin/python
RUN git clone --quiet --branch v0.30.2 https://gitlab.com/drobilla/serd.git
WORKDIR serd
RUN git submodule update --quiet --init --recursive && \
    ./waf configure --static && \
    ./waf install
RUN rm /usr/bin/python
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
RUN conan install . --build=missing --profile default > conan_build.log

# import project files
WORKDIR /tentris
COPY thirdparty thirdparty
COPY src src
COPY cmake cmake
COPY CMakeLists.txt CMakeLists.txt
COPY conanfile.txt conanfile.txt

##build
WORKDIR /tentris/build
# TODO: wait for patch for restinio
# todo: should be replaced with toolchain file like https://github.com/ruslo/polly/blob/master/clang-libcxx17-static.cmake
RUN cmake \
    -DCMAKE_EXE_LINKER_FLAGS="${CMAKE_EXE_LINKER_FLAGS}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DTENTRIS_BUILD_WITH_TCMALLOC=true \
    -DTENTRIS_STATIC=true \
    ..
RUN make -j $(nproc)
FROM scratch
COPY --from=builder /tentris/build/bin/tentris_server /tentris_server
COPY --from=builder /tentris/build/bin/tentris_terminal /tentris_terminal
COPY --from=builder /tentris/build/bin/ids2hypertrie /ids2hypertrie
COPY --from=builder /tentris/build/bin/rdf2ids /rdf2ids
COPY LICENSE LICENSE
COPY README.MD README.MD
ENTRYPOINT ["/tentris_server"]
