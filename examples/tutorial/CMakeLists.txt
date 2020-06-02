add_executable(tutorial_01 01_strong_type.cc)
target_link_libraries(tutorial_01 PRIVATE ezy_lib)

set_target_properties(tutorial_01
  PROPERTIES
    CXX_STANDARD 17
)
