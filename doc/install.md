# Installing ezy

## Installing to a custom directory

After checked out the repository, run the following commands:

```
cmake -B build -DEZY_BUILD_TESTS=OFF -DEZY_BUILD_EXAMPLES
cmake --install build --prefix /my/custom/folder
```

CMake's `find_package` can be used from now:

```cmake
list(APPEND CMAKE_PREFIX_PATH /my/custom/folder)
find_package(ezy REQUIRED)

# ...

target_link_libraries(mytarget PRIVATE ezy::ezy)
```

Since ezy is header-only at the moment, any build system can use it by including the `/my/custom/folder/include`
directory.

## Using `FetchContent`

```cmake
FetchContent_Declare(ezy
  GIT_REPOSITORY https://github.com/titapo/ezy
  GIT_TAG 427e5b3f63ad0d6f3553bdaafcb93bf4d52d8010
)

FetchContent_MakeAvailable(ezy)

# ...

target_link_libraries(mytarget PRIVATE ezy::ezy)
```

## Next steps

Now you are ready to [use ezy](tutorial/).
