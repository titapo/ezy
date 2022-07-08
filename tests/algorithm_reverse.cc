
#include <ezy/algorithm/reverse.h>

#include "join_as_strings.h"

#include <catch2/catch.hpp>

#include <forward_list>

SCENARIO("reverse")
{
  GIVEN("an empty container")
  {
    std::vector<int> empty;
    WHEN("reversed")
    {
      const auto reversed = ezy::reverse(empty);
      THEN("it is empty")
      {
        REQUIRE(ezy::empty(reversed));
      }
    }
  }

  GIVEN("an container with two elements")
  {
    std::vector<int> two{4, 2};
    WHEN("reversed")
    {
      const auto reversed = ezy::reverse(two);
      THEN("its order is reversed")
      {
        REQUIRE(join_as_strings(reversed, ",") == "2,4");
      }
    }
  }

  GIVEN("an container with odd elements")
  {
    std::vector<int> odd{1, 2, 3, 4, 5};
    WHEN("reversed")
    {
      const auto reversed = ezy::reverse(odd);
      THEN("its order is reversed")
      {
        REQUIRE(join_as_strings(reversed, ",") == "5,4,3,2,1");
      }
    }
  }

  GIVEN("an container with some elements")
  {
    std::vector<int> original{1, 2, 3};
    WHEN("reversed")
    {
      auto reversed = ezy::reverse(original);
      AND_WHEN("the reversed element changed")
      {
        *begin(reversed) += 5;
        THEN("the original's last element changed")
        {
          REQUIRE(join_as_strings(original, ",") == "1,2,8");
        }
      }
    }
  }

  GIVEN("an in-place reversed container")
  {
    const auto reversed = ezy::reverse(std::vector<int>{3, 4, 5});
    REQUIRE(join_as_strings(reversed, ",") == "5,4,3");
  }

  GIVEN("an in-place reversed container")
  {
    auto reversed = ezy::reverse(std::vector<int>{3, 4, 5});
    WHEN("its mutated")
    {
      *begin(reversed) += 5;
      THEN("its changed in range")
      {
        REQUIRE(join_as_strings(reversed, ",") == "10,4,3");
      }
    }
  }

  GIVEN("a classic array")
  {
    int numbers[] = {1, 2, 3, 4};
    WHEN("reversed")
    {
      const auto reversed = ezy::reverse(numbers);
      THEN("its element reversed")
      {
        REQUIRE(join_as_strings(reversed, ",") == "4,3,2,1");
      }
    }

    WHEN("(non-const) reversed and modified")
    {
      auto reversed = ezy::reverse(numbers);
      *std::begin(reversed) += 5;
      THEN("its element reversed and modified")
      {
        REQUIRE(join_as_strings(reversed, ",") == "9,3,2,1");
      }
    }
  }

  // It must not compile
  //GIVEN("a forward list")
  //{
  //  std::forward_list<int> forward{1,2,3,4};
  //  THEN("it should not be reversible")
  //  {
  //    auto reversed = ezy::reverse(forward);
  //      REQUIRE(join_as_strings(reversed, ",") == "4,3,2,0");
  //  }
  //}
}
