#include <catch2/catch.hpp>

#include <ezy/dereference.h>

SCENARIO("dereference operator function")
{
  GIVEN("a pointer to a value")
  {
    int i = 10;
    int* p = &i;
    WHEN("dereferenced")
    {
      int value = ezy::dereference(p);
      THEN("its value is the same as the pointed")
      {
        REQUIRE(value == 10);
      }
    }
  }
}

SCENARIO("dereference operator allows mutating the value")
{
  GIVEN("a pointer to a value")
  {
    int i = 11;
    int* p = &i;
    WHEN("dereferenced value incremented")
    {
      ezy::dereference(p) += 5;
      THEN("the original variable changed")
      {
        REQUIRE(i == 16);
      }
    }
  }
}
