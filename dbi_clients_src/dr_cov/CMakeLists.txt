cmake_minimum_required(VERSION 2.6)

if (${USE_COLOR})
    add_definitions(-DUSE_COLOR)
endif()

project(binafl)

set(DynamoRIO_LOG_COMPATIBILITY on)

find_package(DynamoRIO)
if (NOT DynamoRIO_FOUND)
  message(FATAL_ERROR "DynamoRIO package required to build")
endif(NOT DynamoRIO_FOUND)

if (STATIC_LIBRARY)
  set(libtype STATIC)
else()
  set(libtype SHARED)
endif ()

add_library(binafl ${libtype}
  bin_coverage.c
  modules.c
  )

configure_DynamoRIO_client(binafl)
use_DynamoRIO_extension(binafl drmgr)
use_DynamoRIO_extension(binafl drx)
use_DynamoRIO_extension(binafl drcontainers)
use_DynamoRIO_extension(binafl drreg)
use_DynamoRIO_extension(binafl drwrap)
use_DynamoRIO_extension(binafl drsyms)

project(afl_test)

add_executable(afl_test test.c)
