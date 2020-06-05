add_executable(tutorial_01 01_strong_type.cc)
add_executable(tutorial_02 02_iterable_feature.cc)

target_link_libraries(tutorial_01 PRIVATE ezy_lib)
target_link_libraries(tutorial_02 PRIVATE ezy_lib)

set_property(
  TARGET
    tutorial_01
    tutorial_02
  PROPERTY
    CXX_STANDARD 17
  )

# set_target_properties(tutorial_01
#   PROPERTIES
#     CXX_STANDARD 17
# )
