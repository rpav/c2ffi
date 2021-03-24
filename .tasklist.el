((tasks
  (build
   (:name "Build %p")
   (:window "Build/Run %p")
   (:command "grunt" "cmake_build --current-only"))
  (run
   (:name "Run %p")
   (:window "Build/Run %p")
   (:command "grunt" "cmake_build_run --target=c2ffi"))
  (clean
   (:name "Clean %p")
   (:window "Build/Run %p")
   (:command "grunt" "cmake_clean --current-only"))
  (cmake
   (:name "Configure CMake")
   (:window "Build/Run %p")
   (:command "grunt" "cmake_config --current-only"))
  (cmake-clear-cache
   (:name "Clear Cache and Configure CMake")
   (:window "Build/Run %p")
   (:command "grunt" "cmake_config --clear-cache --current-only"))))
