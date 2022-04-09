include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.7
)

# FetchContent_MakeAvailable(Catch2)
# Workaround for old CMake version
FetchContent_GetProperties(Catch2)
if(NOT catch2_POPULATED)
  FetchContent_Populate(Catch2)
  add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR})
endif()

add_executable(unit_test
  main.cc
  ezy.cc
  invoke.cc
  keeper.cc
  math.cc
  strong_type_traits.cc
  tuple_traits.cc
  algorithm.cc
  iterable_feature.cc
  nullable_feature.cc
  to_string.cc
  custom_finder.cc
)

target_link_libraries(unit_test
  PRIVATE
    ezy
    Catch2::Catch2
)

set_target_properties(unit_test
  PROPERTIES
    CXX_STANDARD 17
)

function(ezy_target_add_sanitizer tgt sanitizer)
    target_compile_options(${tgt} PRIVATE -fsanitize=${sanitizer})
    target_link_options(${tgt} PRIVATE -fsanitize=${sanitizer})
endfunction()

if (EZY_SANITIZER)
  ezy_target_add_sanitizer(unit_test ${EZY_SANITIZER})
endif()


target_compile_options(unit_test PRIVATE -pedantic -Wall -Werror)
# target_compile_options(unit_test PRIVATE -D_GLIBCXX_DEBUG)
