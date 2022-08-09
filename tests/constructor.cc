#include <catch2/catch.hpp>

#include <ezy/constructor.h>

#include <vector>

SCENARIO("construct works with std::vector")
{
  WHEN("vector default constructed")
  {
    const auto vector = ezy::construct<std::vector<int>>();
    THEN("result is empty")
    {
      REQUIRE_THAT(vector, Catch::Matchers::Contains(std::vector<int>{}));
    }
  }

  WHEN("vector (2, 3) constructed")
  {
    const auto vector = ezy::construct<std::vector<int>>(2, 3); // 3, 3
    THEN("result contains {3, 3}")
    {
      REQUIRE_THAT(vector, Catch::Matchers::Contains(std::vector<int>{3, 3}));
    }
  }
}

SCENARIO("brace constructor works with std::vector")
{
  WHEN("vector {2, 3} constructed")
  {
    const auto vector = ezy::brace_construct<std::vector<int>>(2, 3); // 2, 3
    THEN("result contains {2, 3}")
    {
      REQUIRE_THAT(vector, Catch::Matchers::Contains(std::vector<int>{2, 3}));
    }
  }
}
