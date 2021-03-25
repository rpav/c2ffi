# This sets up appropriate warnings and some flag tweaks to build correctly
#
# -std= and /std: now handled by cxx_features.cmake
# -lc++abi ?

# reset these in case of from-cmake config

if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  function(SetupPost)
  endfunction()
  return()
endif()

set(BUILD_CONFIG Release CACHE STRING "Build configuration")
set_property(CACHE BUILD_CONFIG PROPERTY STRINGS Debug Release Sanitize)

set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
set(CMAKE_CXX_FLAGS_RELEASE "")

set(DEFAULT_SANITIZERS "address")

if(TOOLCHAIN STREQUAL "clang")
  if(WIN32)
    set(CMAKE_C_COMPILER "clang-cl" FORCE)
    set(CMAKE_CXX_COMPILER "clang-cl" FORCE)
  else()
    set(CMAKE_C_COMPILER clang CACHE STRING "Set compiler to clang" FORCE)
    set(CMAKE_CXX_COMPILER clang++ CACHE STRING "Set compiler to clang" FORCE)
    set(CXX_STDLIB "libstdc++" CACHE STRING "" FORCE)
  endif()
elseif(TOOLCHAIN STREQUAL "gcc")
  set(CMAKE_C_COMPILER gcc CACHE STRING "Set compiler to gcc" FORCE)
  set(CMAKE_CXX_COMPILER g++ CACHE STRING "Set compiler to gcc" FORCE)
  set(CXX_STDLIB "libstdc++" CACHE STRING "" FORCE)
endif()

if(BUILD_CONFIG STREQUAL "Debug")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
elseif(BUILD_CONFIG STREQUAL "Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
elseif(BUILD_CONFIG STREQUAL "Sanitize")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)

  if("${ENABLE_SANITIZER}" STREQUAL "")
    message(STATUS "Enabling default sanitizers (${DEFAULT_SANITIZERS})")
    set(ENABLE_SANITIZER "${DEFAULT_SANITIZERS}" CACHE STRING "Default sanitizer" FORCE)
  endif()
else()
  message(FATAL_ERROR "BUILD_CONFIG not recognized: '${BUILD_CONFIG}'")
endif()

message(STATUS "Config: ${TOOLCHAIN}-${BUILD_CONFIG}")

if(BUILD_CONFIG STREQUAL "Debug" AND (NOT ENABLE_PCH))
  if(TOOLCHAIN MATCHES "^(clang)$")
    message(STATUS "Enabling compile_commands.json")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile_commands.json" FORCE)
  endif()
endif()

set(CMAKE_PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/setup_post_project.cmake")

function(target_link_modules TARGET)
  set(multiValueArgs PUBLIC PRIVATE)
  cmake_parse_arguments(m "" "" "${multiValueArgs}" ${ARGN})

  foreach(target IN LISTS m_PUBLIC m_PRIVATE)
    if(NOT TARGET ${target})
      message(FATAL_ERROR "${TARGET} attempts to link ${target}, which is not a valid module")
    endif()
  endforeach()

  target_link_libraries(${TARGET} PUBLIC ${m_PUBLIC} PRIVATE ${m_PRIVATE})
endfunction()
