
#include <ezy/math.h>

#include <catch2/catch.hpp>

SCENARIO("max")
{
  WHEN("max is called with two integers")
  {
    auto larger = ezy::max(12, 21);
    THEN("larger is returned")
    {
      REQUIRE(larger == 21);
    }
  }

  WHEN("max is passed as a predicate")
  {
    auto list = {1, 5, 29, 42, 100, 70, 23};
    auto max_found = std::accumulate(std::begin(list), std::end(list), 0, ezy::max);
    REQUIRE(max_found == 100);
  }

  WHEN("max is called with initializer list")
  {
    REQUIRE(ezy::max({4, 25, 7}) == 25);
  }

  // TODO think of max_by
}

SCENARIO("min")
{
  WHEN("min is called with two integers")
  {
    auto smaller = ezy::min(12, 21);
    THEN("smaller is returned")
    {
      REQUIRE(smaller == 12);
    }
  }

  WHEN("min is passed as a predicate")
  {
    auto list = {1, 5, 29, 42, 100, 70, 23};
    auto min_found = std::accumulate(std::begin(list), std::end(list), 1000, ezy::min);
    REQUIRE(min_found == 1);
  }

  WHEN("min is called with initializer list")
  {
    REQUIRE(ezy::min({4, 25, 7}) == 4);
  }
  // TODO think of min_by
}

SCENARIO("abs")
{
  // In clang abs() is not marked as constexpr
  // static_assert(ezy::abs(-12) == 12);

  WHEN("used with integer")
  {
    REQUIRE(ezy::abs(10) == 10);
    REQUIRE(ezy::abs(-10) == 10);
  }

  WHEN("used with long")
  {
    REQUIRE(ezy::abs(10000000000000l) == 10000000000000l);
    REQUIRE(ezy::abs(-100000000000000l) == 100000000000000l);
  }

  WHEN("used with double")
  {
    REQUIRE(ezy::abs(123.45) == 123.45);
    REQUIRE(ezy::abs(-123.45) == 123.45);
  }

  WHEN("used as a higher order function")
  {
    auto list = {1, -2, 3, -4};
    std::vector<int> out(4);
    std::transform(std::begin(list), std::end(list), std::begin(out), ezy::abs);
    auto expected = {1, 2, 3, 4};
    REQUIRE(std::equal(std::begin(out), std::end(out), std::begin(expected)));
  }
}

SCENARIO("clamp")
{
  WHEN("value is between low and high")
  {
    REQUIRE(ezy::clamp(10, 20)(15) == 15);
  }

  WHEN("value is lesser than low")
  {
    REQUIRE(ezy::clamp(10, 20)(5) == 10);
  }

  WHEN("value is greater than high")
  {
    REQUIRE(ezy::clamp(10, 20)(25) == 20);
  }
}
