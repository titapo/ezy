add_executable(unit_test main.cc ezy.cc strong_type_traits.cc tuple_traits.cc iterable_feature.cc)

# include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/thirdparty/catch/include)
target_include_directories(unit_test PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/thirdparty/catch/include)

#target_link_libraries(unit_test PRIVATE ezy_lib catch)

set_property(TARGET unit_test PROPERTY CXX_STANDARD 17)

target_compile_options(unit_test PRIVATE -pedantic -Wall -Werror)


# add_test(NAME unit_test COMMAND check)
