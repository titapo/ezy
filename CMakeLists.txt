cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(EZY_IS_TOP_LEVEL ON)
else()
  set(EZY_IS_TOP_LEVEL OFF)
endif ()

project(ezy
  LANGUAGES CXX
  VERSION 0.0.1
)

option(EZY_BUILD_TESTS "Build ezy tests" ON)
option(EZY_BUILD_EXAMPLES "Build ezy examples" ON)

message(STATUS "Configuring: ezy (${PROJECT_VERSION})")
if (EZY_IS_TOP_LEVEL)
  message(STATUS "  EZY_BUILD_TESTS=${EZY_BUILD_TESTS}")
  message(STATUS "  EZY_BUILD_EXAMPLES=${EZY_BUILD_EXAMPLES}")
endif ()

add_library(ezy INTERFACE)
add_library(ezy::ezy ALIAS ezy)

target_include_directories(ezy
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

if (EZY_BUILD_TESTS AND EZY_IS_TOP_LEVEL)
  add_subdirectory(tests)
endif ()

if (EZY_BUILD_EXAMPLES AND EZY_IS_TOP_LEVEL)
  add_subdirectory(examples)
endif ()

## 
## # testing
## enable_testing()
## add_subdirectory( tests )
## 

if (EZY_IS_TOP_LEVEL)
  install(
    TARGETS
      ezy
    EXPORT
      EzyTargets
    DESTINATION
      ${CMAKE_INSTALL_LIBDIR}
  )

  install(
    EXPORT
      EzyTargets
    NAMESPACE
      ezy::
    DESTINATION
      lib/cmake/ezy
  )

  install(
    DIRECTORY include/ezy
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  include(CMakePackageConfigHelpers)

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ezy-config.cmake
    INSTALL_DESTINATION lib/cmake/ezy
  )

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ezy-config-version.cmake
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY AnyNewerVersion
  )

  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/ezy-config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/ezy-config-version.cmake
    DESTINATION
      lib/cmake/ezy
  )


endif ()
