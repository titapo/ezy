add_executable(01_feature 01_feature.cc)
target_link_libraries(01_feature PRIVATE ezy)

set_property(
  TARGET
    01_feature
  PROPERTY
    CXX_STANDARD 17
  )
