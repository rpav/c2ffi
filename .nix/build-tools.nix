{ toolchain, ... }:
with import <unstable> {};
let
  # These should be the _latest_, from which we pull tools
  llvm        = llvmPackages_14;
  clang-tools = clang-tools_14;
in rec {
  toolchains = with pkgs; {
    gcc-12 = [
      gcc12
      clang-tools
    ];

    clang-14 = [
      clang-tools_14
    ];

    clang-15 = [
      clang-tools_15
    ];
  };

  envs = with pkgs; {
    gcc-12 = pkgs.stdenv;
    clang-14 = llvmPackages_14.stdenv;
    clang-15 = llvmPackages_15.stdenv;
  };

  common = with pkgs; [
    cmake
    ninja
    llvm.lld
  ];

  # Composed packages based on toolchain
  nativeBuildInputs = (builtins.getAttr toolchain toolchains) ++ common;
  buildInputs = [];

  stdenv = (builtins.getAttr toolchain envs);
}
