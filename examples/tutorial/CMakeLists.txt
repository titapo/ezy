add_executable(tutorial_01 01_strong_type.cc)
add_executable(tutorial_02 02_iterable_feature.cc)
add_executable(tutorial_03 03_extended_type.cc)
add_executable(tutorial_04 04_algorithm.cc)
add_executable(tutorial_05 05_optional.cc)

target_link_libraries(tutorial_01 PRIVATE ezy_lib)
target_link_libraries(tutorial_02 PRIVATE ezy_lib)
target_link_libraries(tutorial_03 PRIVATE ezy_lib)
target_link_libraries(tutorial_04 PRIVATE ezy_lib)
target_link_libraries(tutorial_05 PRIVATE ezy_lib)

set_property(
  TARGET
    tutorial_01
    tutorial_02
    tutorial_03
    tutorial_04
    tutorial_05
  PROPERTY
    CXX_STANDARD 17
  )
