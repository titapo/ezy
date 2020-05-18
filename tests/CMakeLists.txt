add_executable(unit_test
  main.cc
  ezy.cc
  invoke.cc
  keeper.cc
  strong_type_traits.cc
  tuple_traits.cc
  iterable_feature.cc
  algorithm.cc
  nullable_feature.cc
  to_string.cc
)

target_link_libraries(unit_test
  PRIVATE
    ezy_lib
    catch
)

set_target_properties(unit_test
  PROPERTIES
    CXX_STANDARD 17
)

target_compile_options(unit_test PRIVATE -pedantic -Wall -Werror)
# target_compile_options(unit_test PRIVATE -D_GLIBCXX_DEBUG)
