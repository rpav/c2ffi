((cmake-build-cmake-profiles
  (clang-debug "-C ~/.cmake/clang -C ~/.cmake/debug"))
 (cmake-build-run-configs
  (c2ffi
   (:build "c2ffi")
   (:run "bin" "./c2ffi" ""))))
