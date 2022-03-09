cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(ezy LANGUAGES CXX)

add_library(ezy_lib INTERFACE)
target_include_directories(ezy_lib
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

add_subdirectory(tests)
add_subdirectory(examples)

## 
## # testing
## enable_testing()
## add_subdirectory( tests )
## 
install(DIRECTORY include/ezy DESTINATION include)
