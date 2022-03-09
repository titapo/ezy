add_executable(01_strong_type      01_strong_type.cc)
add_executable(02_features         02_features.cc)
add_executable(03_iterable_feature 03_iterable_feature.cc)
add_executable(04_extended_type    04_extended_type.cc)
add_executable(05_algorithm        05_algorithm.cc)
add_executable(06_optional         06_optional.cc)
add_executable(07_result           07_result.cc)
add_executable(08_nullable         08_nullable.cc)

target_link_libraries(01_strong_type      PRIVATE ezy)
target_link_libraries(02_features         PRIVATE ezy)
target_link_libraries(03_iterable_feature PRIVATE ezy)
target_link_libraries(04_extended_type    PRIVATE ezy)
target_link_libraries(05_algorithm        PRIVATE ezy)
target_link_libraries(06_optional         PRIVATE ezy)
target_link_libraries(07_result           PRIVATE ezy)
target_link_libraries(08_nullable         PRIVATE ezy)

set_property(
  TARGET
    01_strong_type
    02_features
    03_iterable_feature
    04_extended_type
    05_algorithm
    06_optional
    07_result
    08_nullable
  PROPERTY
    CXX_STANDARD 17
  )
