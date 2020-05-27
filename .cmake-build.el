((cmake-build-cmake-profiles
  (clang-Release "-DTOOLCHAIN=clang -DCMAKE_BUILD_TYPE=Release")
  (clang-Debug "-DTOOLCHAIN=clang -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
  (clang-Sanitize "-DTOOLCHAIN=clang -DCMAKE_BUILD_TYPE=Release -DENABLE_SANITIZER=address,undefined")
  (gcc-Release "-DTOOLCHAIN=gcc -DCMAKE_BUILD_TYPE=Release")
  (gcc-Sanitize "-DTOOLCHAIN=gcc -DCMAKE_BUILD_TYPE=Sanitize")
  (gcc-Debug "-DTOOLCHAIN=gcc -DCMAKE_BUILD_TYPE=Debug"))
 (cmake-build-run-configs
  (c2ffi
   (:build "c2ffi")
   (:run "" "./bin/c2ffi" "/usr/include/stdint.h"))))
