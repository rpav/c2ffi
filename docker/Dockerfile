# after building, run with:
# docker run --rm -ti c2ffi c2ffi

FROM ubuntu:xenial
MAINTAINER Stathis Sideris

ENV llvm_release 3.7

RUN apt-get -y update && apt-get -y upgrade

#Utilities
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update -y \
 && apt-get install -y \
    clang-${llvm_release} \
    clang-${llvm_release}-doc \
    libclang-common-${llvm_release}-dev \
    libclang-${llvm_release}-dev \
    libclang1-${llvm_release} \
    libllvm-${llvm_release}-ocaml-dev \
    libllvm${llvm_release} \
    lldb-${llvm_release} \
    llvm-${llvm_release} \
    llvm-${llvm_release}-dev \
    llvm-${llvm_release}-doc \
    llvm-${llvm_release}-examples \
    llvm-${llvm_release}-runtime \
    clang-modernize-${llvm_release} \
    clang-format-${llvm_release} \
    python-clang-${llvm_release} \
    lldb-${llvm_release}-dev \
    zlib1g-dev \
    libedit-dev \
    cmake \
    build-essential \
    git

RUN git clone https://github.com/rpav/c2ffi.git \
 && cd c2ffi \
# && git checkout -b llvm-${llvm_release}
 && mkdir build \
 && cd build \
 && cmake .. \
 && make -j4 VERBOSE=1

#Clean up
RUN apt-get clean \
 && rm -rf /var/lib/apt/lists/*

RUN mv c2ffi/build/bin/c2ffi /usr/local/bin/
