# This sets up appropriate warnings and some flag tweaks to build correctly
#
# -std= and /std: now handled by cxx_features.cmake
# -lc++abi should be handled by conan?

# reset these in case of from-cmake config

if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  function(SetupPost)
  endfunction()
  return()
endif()

set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
set(CMAKE_CXX_FLAGS_RELEASE "")

if(TOOLCHAIN STREQUAL "clang")
  if(WIN32)
    set(CMAKE_C_COMPILER "clang-cl" FORCE)
    set(CMAKE_CXX_COMPILER "clang-cl" FORCE)
  else()
    set(CMAKE_C_COMPILER clang CACHE STRING "Set compiler to clang" FORCE)
    set(CMAKE_CXX_COMPILER clang++ CACHE STRING "Set compiler to clang" FORCE)
    #set(CXX_STDLIB "libc++" CACHE STRING "" FORCE)
  endif()
elseif(TOOLCHAIN STREQUAL "gcc")
  set(CMAKE_C_COMPILER gcc CACHE STRING "Set compiler to gcc" FORCE)
  set(CMAKE_CXX_COMPILER g++ CACHE STRING "Set compiler to gcc" FORCE)
  set(CXX_STDLIB "libstdc++11" CACHE STRING "" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if(TOOLCHAIN MATCHES "^(clang)$")
    message(STATUS "Enabling compile_commands.json")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile_commands.json" FORCE)
  endif()
endif()

set(CMAKE_PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/setup_post_project.cmake")
