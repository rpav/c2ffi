# to enter the environment: nix-shell
# to build: nix-build
#   nix-build --option repeat 1 to check if the build is reproducible
#   nix-build --arg llvmFromUnstable true to build with LLVM from unstable

{ llvmFromUnstable ? false }:

let
  pkgs = if llvmFromUnstable
         then import (fetchTarball https://github.com/NixOS/nixpkgs/archive/nixpkgs-unstable.tar.gz) {}
         else import <nixpkgs> {};

  branch = "11.0.0";
  llvmPackages = pkgs.llvmPackages_11;

in

llvmPackages.stdenv.mkDerivation {
  version = "unstable";

  pname = "c2ffi-" + branch;

  src = ./.;

  nativeBuildInputs = with pkgs; [
    cmake
  ];

  buildInputs = with pkgs; [
    llvmPackages.llvm
    llvmPackages.clang
    llvmPackages.libclang.out
  ];

  # this is needed when compiling with LLVM 11.1.0 (from unstable at 2021-04-14)
  CXXFLAGS="-fno-rtti";

  shellHook =
  ''
    alias ..='cd ..'
    alias ...='cd ../..'
  '';
}
