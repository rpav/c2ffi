# after building, run with:
# docker run --rm -ti c2ffi c2ffi

FROM ubuntu:xenial
MAINTAINER Stathis Sideris

ENV llvm_release 3.7

RUN apt-get -y update && apt-get -y upgrade

#Utilities
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update -y \
 && apt-get install -y --force-yes \
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
    cmake \
    build-essential \
    vim \
    less \
    curl \
    git \
    wget \
    xz-utils \
    cmake \
    python \
    build-essential \
    m4 \
    autoconf \
    automake \
    libtool \
    zlib1g-dev \
    binutils \
    libedit-dev

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
