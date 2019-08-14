((cmake-build-cmake-profiles
  (clang-debug "-C ~/.cmake/clang-8.0 -C ~/.cmake/debug"))
 (cmake-build-run-configs
  (c2ffi
   (:build "c2ffi")
   (:run "bin" "./c2ffi" ""))))
