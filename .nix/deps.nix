{...}:
with import <nixpkgs> {};
{
  buildInputs = [
    llvmPackages_14.llvm
    llvmPackages_14.libclang
  ];
}
