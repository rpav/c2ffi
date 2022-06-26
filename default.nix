# to enter the environment:
#   nix-shell
# to build:
#   nix-build
#   nix-build --option repeat 1             # to check if the build is reproducible
#   nix-build --arg llvmFromUnstable true   # to build with LLVM from unstable
#   nix-build --argstr rev 80a298a9c63f5b05c16a614b309c2414d439388b --argstr sha256 17rh1wjvpz9xqm6c8x7j23jsysaw3bdzfkkvlx91mqfblalcy164

{
  llvmFromUnstable ? false
, rev ? null
, sha256 ? null
}:

let
  pkgs = if llvmFromUnstable
         then import (fetchTarball https://github.com/NixOS/nixpkgs/archive/nixpkgs-unstable.tar.gz) {}
         else import <nixpkgs> {};

  c2ffiBranch = "llvm-12.0.0";
  llvmPackages = pkgs.llvmPackages_12;
in

llvmPackages.stdenv.mkDerivation {
  version = if rev == null then
              "unstable"
            else
              "g" + builtins.substring 0 9 rev; # this seems to be some kind of standard of git describe --tags HEAD

  pname = "c2ffi-" + c2ffiBranch;

  src = if rev == null then
          ./.
        else
          pkgs.fetchgit {
            url = ./.;
            rev = rev;
            sha256 = sha256;
          };

  nativeBuildInputs = with pkgs; [
    cmake
  ];

  buildInputs = with pkgs; [
    llvmPackages.llvm
    llvmPackages.clang
    llvmPackages.libclang
  ];

  # this is needed when compiling with LLVM 11.1.0 (from unstable at 2021-04-14)
  CXXFLAGS="-fno-rtti";

  shellHook =
  ''
    alias ..='cd ..'
    alias ...='cd ../..'
  '';
}
