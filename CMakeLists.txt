cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(ezy LANGUAGES CXX)

option(EZY_BUILD_TESTS "Build ezy tests" ON)
option(EZY_BUILD_EXAMPLES "Build ezy examples" ON)

add_library(ezy_lib INTERFACE)
target_include_directories(ezy_lib
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

if (EZY_BUILD_TESTS)
  add_subdirectory(tests)
endif ()

if (EZY_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif ()

## 
## # testing
## enable_testing()
## add_subdirectory( tests )
## 
install(DIRECTORY include/ezy DESTINATION include)
