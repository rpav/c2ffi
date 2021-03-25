##
## Pick C++ version and features for compiler
##
## Why: Compilers require different flags for C++ standard conformance, as well
## as some requiring libraries linked for various functionality (and some later
## versions *lack* these libraries).  This is an easy/extensible solution for
## such, since CMake's builtin picking is undependable.
##
## Usage:
##
##   target_cxx_std(TARGET VERSION)
##
##     VERSION should be the C++ standard, e.g. 11, 17, 20, ..
##
##   target_cxx_features(TARGET FEATURE...)
##
##     FEATURE should be {filesystem, threads}
##
##
## Copyright (c) 2020 Ryan Pavlik
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to
## deal in the Software without restriction, including without limitation the
## rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
## sell copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in
## all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
## AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
## FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
## IN THE SOFTWARE.


cmake_minimum_required(VERSION 3.11)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  if(WIN32 AND NOT MINGW)
    function(__cxx_feature_process)
      __cxx_feature_clang_cl(${ARGN})
    endfunction()
    macro(__cxx_std_process)
      __cxx_std_msvc(${ARGN})
    endmacro()
  else()
    if(CXX_STDLIB STREQUAL "libc++")
      if(NOT CMAKE_CXX_FLAGS MATCHES "-stdlib=libc\\+\\+")
        message(STATUS "Setting -std=libc++")
        set(CMAKE_CXX_FLAGS "-stdlib=libc++ ${CMAKE_CXX_FLAGS}")
      endif()

      function(__cxx_feature_process)
        __cxx_feature_clang_libcxx(${ARGN})
      endfunction()
    else()
      function(__cxx_feature_process)
        __cxx_feature_gnu(${ARGN})
      endfunction()
    endif()
    macro(__cxx_std_process)
      __cxx_std_gnu(${ARGN})
    endmacro()
  endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  function(__cxx_feature_process)
    __cxx_feature_gnu(${ARGN})
  endfunction()
  macro(__cxx_std_process)
    __cxx_std_gnu(${ARGN})
  endmacro()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  function(__cxx_feature_process)
    __cxx_feature_msvc(${ARGN})
  endfunction()
  macro(__cxx_std_process)
    __cxx_std_msvc(${ARGN})
  endmacro()
endif ()

function(target_cxx_features TARGET)
  foreach(feature IN LISTS ARGN)
    __cxx_feature_process(${TARGET} ${feature})
  endforeach()
endfunction()

function(target_cxx_std TARGET STD)
  set(oneValueArgs EXTENSIONS)
  cmake_parse_arguments(_ "" "${oneValueArgs}" "" ${ARGN})

  get_target_property(type ${TARGET} TYPE)

  if(NOT _EXTENSIONS)
    set(_EXTENSIONS OFF)
  endif()

  __cxx_std_process(_std_flag "${STD}" "${_EXTENSIONS}")
  message(STATUS "${TARGET} c++std: ${_std_flag}")

  if(type STREQUAL "INTERFACE_LIBRARY")
    target_compile_options(${TARGET} INTERFACE ${_std_flag})
  else()
    target_compile_options(${TARGET} PUBLIC ${_std_flag})
  endif()
endfunction()


function(__cxx_feature_clang_cl T F)
endfunction()

function(__cxx_feature_clang_libcxx T F)
  if(F STREQUAL "filesystem")
    if(CLANG_VERSION_MINOR LESS 9)
      target_link_libraries(${T} INTERFACE c++fs)
    endif()
  elseif(F STREQUAL "threads")
    target_compile_options(${T} PUBLIC "-pthread")
  endif()
endfunction()

function(__cxx_feature_gnu T F)
  if(F STREQUAL "filesystem")
    target_link_libraries(${T} INTERFACE stdc++fs)
  elseif(F STREQUAL "threads")
    target_compile_options(${T} PUBLIC "-pthread")
  endif()
endfunction()

function(__cxx_feature_msvc T F)
endfunction()

## -std=

macro(__cxx_std_gnu VAR VER EXT)
  if(EXT)
    set(prefix "gnu")
  else()
    set(prefix "c++")
  endif()

  set(${VAR} "-std=${prefix}${VER}")
endmacro()

macro(__cxx_std_msvc VAR VER EXT)
  if(EXT)
    message(FATAL "Extensions not supported by MSVC")
  endif()

  if(VER LESS_EQUAL 14)
    set(ver 14)
  elseif(VER EQUAL 17)
    set(ver 17)
  else()
    set(ver latest)
  endif()

  set(${VAR} "/std:c++${ver}")
endmacro()
