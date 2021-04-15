#!/usr/bin/env bash
set -e
set -x
bindir="$1"
testdir="$2"
C2FFI="$(realpath $bindir/c2ffi)"

rm -rf "$testdir/out"
mkdir -p "$testdir/out"
cd "$testdir"
"$C2FFI" -T "out/test1.tem.hh" -M "out/test1.mac.hh" -D null "test1.hh"
echo "#include \"out/test1.mac.hh\"" >> "out/test1.tem.hh"
"$C2FFI" "out/test1.tem.hh" -D json -o "out/test1.spec" -I "."
diff "out/test1.spec" "test1.spec"
