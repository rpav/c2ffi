{ toolchain ? "clang-14", project-root ? ".", ... }:
with import <nixpkgs> {};
let
  build-tools = import ./build-tools.nix { toolchain = toolchain; };
  project-path = /. + project-root;
  deps-path = (project-path + /.nix/deps.nix);

  deps = if builtins.pathExists deps-path then
    { buildInputs = []; nativeBuildInputs = []; } // import deps-path {}
  else
    { buildInputs = []; nativeBuildInputs = []; };

in
build-tools.stdenv.mkDerivation {
  name = "gcc-12-toolchain";

  nativeBuildInputs = build-tools.nativeBuildInputs ++ deps.nativeBuildInputs;
  buildInputs = build-tools.buildInputs ++ deps.buildInputs;

  shellHook =
  ''
    echo "toolchain:    ${toolchain}"
    echo "project-root: ${project-root}"
  '';
}
