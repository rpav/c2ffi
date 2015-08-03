cmake_minimum_required(VERSION 3.2)

### Basic config ##########################################################

set(APP_NAME c2ffi)
project(APP_NAME)

set(SOURCE_ROOT ${CMAKE_SOURCE_DIR})


### Modules ###############################################################

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "more...")
# include(stuff...)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs core support)
message(STATUS "Using llvm libs: ${llvm_libs}")

### Variables #############################################################

# set(var_name val CACHE type "Description")

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS    "${CMAKE_CXX_FLAGS} ${LLVM_COMPILE_FLAGS}")

message(STATUS "CXXFLAGS: ${CMAKE_CXX_FLAGS}")
 

### Subdirs ###############################################################

# add_subdirectory(path)


### Source ################################################################

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

add_executable(${APP_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(${APP_NAME}
  ${llvm_libs}
  clangAnalysis
  clangApplyReplacements
  clangARCMigrate
  clangASTMatchers
  clangAST
  clangBasic
  clangCodeGen
  clangDriver
  clangDynamicASTMatchers
  clangEdit
  clangFormat
  clangFrontend
  clangFrontendTool
  clangIndex
  clangLex
  clangParse
  clangQuery
  clangRename
  clangRewriteFrontend
  clangRewrite
  clangSema
  clangSerialization
  clang
  clangStaticAnalyzerCheckers
  clangStaticAnalyzerCore
  clangStaticAnalyzerFrontend
  clangTidyGoogleModule
  clangTidyLLVMModule
  clangTidyMiscModule
  clangTidyReadabilityModule
  clangTidy
  clangTidyUtils
  clangToolingCore
  clangTooling)

set(APP_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set_target_properties(${APP_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${APP_BIN_DIR}"
  )

### Custom Targets ########################################################

# add_custom_target(name
#   COMMAND ...
#   )
