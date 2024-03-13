cmake_minimum_required(VERSION 3.17)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
include(setup)

project(c2ffi)

set(SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Apparently the LLVM package doesn't support ranges
find_package(LLVM 17.0.0 CONFIG)
find_package(Clang REQUIRED CONFIG)

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

# Find Clang resource directory with Clang executable.
if(NOT CLANG_RESOURCE_DIR)
    find_program(CLANG_EXECUTABLE ${CLANG_INSTALL_PREFIX}/bin/clang)
    if(NOT CLANG_EXECUTABLE)
      find_program(CLANG_EXECUTABLE clang)
    endif()
    if(NOT CLANG_EXECUTABLE)
      message(FATAL_ERROR "clang executable not found.")
    endif()

    execute_process(
        COMMAND ${CLANG_EXECUTABLE} -print-resource-dir
        RESULT_VARIABLE CLANG_FIND_RESOURCE_DIR_RESULT
        OUTPUT_VARIABLE CLANG_RESOURCE_DIR
        ERROR_VARIABLE CLANG_FIND_RESOURCE_DIR_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

    if(CLANG_FIND_RESOURCE_DIR_RESULT)
        message(FATAL_ERROR "Error retrieving Clang resource directory with Clang \
            executable. Output:\n ${CLANG_FIND_RESOURCE_DIR_ERROR}")
    endif()
endif()

add_executable(c2ffi ${SOURCE_FILES} ${HEADER_FILES})
set_property(SOURCE src/init.cpp APPEND PROPERTY COMPILE_DEFINITIONS
    CLANG_RESOURCE_DIRECTORY=R"\(${CLANG_RESOURCE_DIR}\)")
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
