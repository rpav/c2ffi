cmake_minimum_required(VERSION 3.17)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
include(setup)

project(c2ffi)

set(SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Apparently the LLVM package doesn't support ranges
find_package(LLVM 11.1.0 CONFIG)
if(NOT LLVM_FOUND)
  find_package(LLVM 11.0.0 REQUIRED CONFIG)
endif()


find_package(Clang)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM installed in ${LLVM_INSTALL_PREFIX}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

file(GLOB SOURCE_GLOB
  ${SOURCE_ROOT}/src/*.cpp
  ${SOURCE_ROOT}/src/drivers/*.cpp
  )

file(GLOB HEADER_GLOB
  ${SOURCE_ROOT}/include/*.h
  ${SOURCE_ROOT}/include/c2ffi/*.h
  )

set(SOURCE_FILES
  ${SOURCE_GLOB}
  )

set(HEADER_FILES
  ${HEADER_GLOB}
  )

add_executable(c2ffi ${SOURCE_FILES} ${HEADER_FILES})
target_cxx_std(c2ffi 17)
target_include_directories(c2ffi PUBLIC
  ${LLVM_INCLUDE_DIRS}
  ${SOURCE_ROOT}/src/include
  )
target_link_libraries(c2ffi PUBLIC clang-cpp LLVM)

set(APP_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set_target_properties(c2ffi PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${APP_BIN_DIR}"
  )

install(TARGETS c2ffi DESTINATION bin)

SetupPost()
