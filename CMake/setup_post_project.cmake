set(gnu_cxx_warnings "-W -Wall -Wextra -Wno-unused-label -Wno-unused-parameter")
set(gnu_cxx_warn_error "-Werror=return-type -Werror=unused-result")

set(clang_cxx_warnings "-W -Wall -Wextra")
set(clang_cxx_warn_error "-pedantic-errors -Werror=return-type -Werror=unused-result")
set(clang_cxx_nowarn "-Wno-unused-parameter -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-unused-label")

set(msvc_debug "/D_DEBUG /MDd /Zi /D_ITERATOR_DEBUG_LEVEL=0 /GL")
set(msvc_release "/MD /GL")
set(msvc_rwdi "/DNDEBUG /MDd /Zi /D_ITERATOR_DEBUG_LEVEL=0 /GL")

set(gnu_debug "-O1 -ggdb2 -fno-omit-frame-pointer")
set(gnu_rwdi "-O2 -ggdb2 -fno-omit-frame-pointer -DNDEBUG")
set(gnu_release "-O3 -DNDEBUG")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(X64 1)
  set(ARCH "x64")
else ()
  set(X86 1)
  set(ARCH "x86")
endif ()

message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER_ID}")

if (WIN32)
  message(STATUS "Building for Win32-${ARCH}")

  if (ENABLE_SANITIZER)
    message(WARNING "ENABLE_SANITIZER specified, but Address Sanitizer not supported in Windows")
  endif ()

  function (__target_sanitize TARGET)
    message(WARNING "(Can't enable sanitizers on ${TARGET}")
  endfunction(__target_sanitize)
else ()
  message(STATUS "Building for Linux-${ARCH}")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
  endif()

  function (__target_sanitize TARGET)
    if(ENABLE_SANITIZER)
      message(STATUS "${TARGET} sanitizers: ${ENABLE_SANITIZER}")
      target_compile_options(${TARGET} BEFORE PUBLIC "-fsanitize=${ENABLE_SANITIZER}")
      target_link_options(${TARGET} BEFORE PUBLIC "-fsanitize=${ENABLE_SANITIZER}")
    endif()
  endfunction(__target_sanitize)

  if (ENABLE_GPROF)
    message(STATUS "Enabling profiling (via gprof)")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
  endif ()
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if (WIN32 AND NOT MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /permissive-")
  else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${clang_cxx_warnings} ${clang_cxx_warn_error} ${clang_cxx_nowarn}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${gnu_debug}")
    set(CMAKE_CXX_FLAGS_RELEASE "${gnu_release}" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${gnu_rwdi}" CACHE STRING "" FORCE)
  endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${gnu_cxx_warnings} ${gnu_cxx_warn_error}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${gnu_debug}")
  set(CMAKE_CXX_FLAGS_RELEASE "${gnu_release}" CACHE STRING "" FORCE)
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${gnu_rwdi}" CACHE STRING "" FORCE)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /permissive-")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${msvc_debug}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${msvc_release}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${msvc_rwdi}" CACHE STRING "" FORCE)

  set(all_linker_flags "/LTCG")

  set(CMAKE_MODULE_LINKER_FLAGS "${all_linker_flags}")
  set(CMAKE_SHARED_LINKER_FLAGS "${all_linker_flags}")
  set(CMAKE_STATIC_LINKER_FLAGS "${all_linker_flags}")
  set(CMAKE_EXE_LINKER_FLAGS "${all_linker_flags}")

  set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "")
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "")
  set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
endif ()


function(SetupPost)
  # Pretty annoying after a decade+ cmake still can't indirectly
  # call functions
  function(ProcessDir DIR)
    get_property(targets DIRECTORY ${DIR} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target ${targets})
      get_target_property(target_type ${target} TYPE)
      if(target_type STREQUAL "EXECUTABLE"
          OR target_type STREQUAL "STATIC_LIBRARY"
          OR target_type STREQUAL "SHARED_LIBRARY")
        __target_sanitize(${target})
      endif()
    endforeach()
  endfunction()

  function(MapDirs DIR)
    ProcessDir(${DIR})
    get_property(subdirs DIRECTORY ${DIR} PROPERTY SUBDIRECTORIES)
    foreach(dir ${subdirs})
      MapDirs(${dir})
    endforeach()
  endfunction()

  MapDirs(${CMAKE_CURRENT_SOURCE_DIR})

  message("**   C++ flags: ${CMAKE_CXX_FLAGS}")
  message("**   C++ debug    flags: ${CMAKE_CXX_FLAGS_DEBUG}")
  message("**   C++ rwdi     flags: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  message("**   C++ release  flags: ${CMAKE_CXX_FLAGS_RELEASE}")
  message("**   Link flags: ${CMAKE_EXE_LINKER_FLAGS}")
  message("**   Link debug flags: ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
  message("**")
  message("**   This build: ${CMAKE_BUILD_TYPE}")
endfunction(SetupPost)

include(cxx_features)
