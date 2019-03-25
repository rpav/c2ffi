cmake_minimum_required(VERSION 3.8)

project(c2ffi)

set(SOURCE_ROOT ${CMAKE_SOURCE_DIR})

find_package(LLVM 7.0 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM installed in ${LLVM_INSTALL_PREFIX}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
add_definitions(${LLVM_DEFINITIONS})
link_directories(${LLVM_INSTALL_PREFIX}/lib ${LLVM_INSTALL_PREFIX}/lib64)

set(CMAKE_CXX_STANDARD 11)

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${SOURCE_ROOT}/src/include
  )

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
target_compile_options(c2ffi PUBLIC -fno-exceptions -fno-rtti)
target_link_libraries(c2ffi PUBLIC
  clangFrontendTool
  clangFrontend
  clangDriver
  clangSerialization
  clangCodeGen
  clangParse
  clangSema
  clangStaticAnalyzerFrontend
  clangStaticAnalyzerCheckers
  clangStaticAnalyzerCore
  clangAnalysis
  clangARCMigrate
  clangRewriteFrontend
  clangRewrite
  clangEdit
  clangDynamicASTMatchers
  clangASTMatchers
  clangAST
  clangLex
  clangBasic
  clangFormat
  clangIndex
  clangToolingCore
  clangTooling
  LLVM)

set(APP_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set_target_properties(c2ffi PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${APP_BIN_DIR}"
  )

install(TARGETS c2ffi DESTINATION bin)
