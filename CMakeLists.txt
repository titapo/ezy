cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(EZY)

add_subdirectory(thirdparty/catch)
add_subdirectory(include)
add_subdirectory(tests)

## # library
## add_library(ezy INTERFACE)
## target_include_directories(ezy INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
## target_include_directories(ezy SYSTEM INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>)
## 
## 
## # testing
## enable_testing()
## add_subdirectory( tests )
## 
## install(DIRECTORY include/ezy DESTINATION include)
