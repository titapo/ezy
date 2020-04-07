#include <catch.hpp>

#include <ezy/invoke.h>

int foo()
{
  return 101;
}

SCENARIO("invoke")
{
  GIVEN("a struct")
  {
    struct S
    {
      int f() { return 3; };
      constexpr int cf() const { return 4; }

      int mem{5};
    };

    struct D : S
    {};

    S s;

    WHEN("its pointer to member function invoked")
    {
      REQUIRE(ezy::invoke(&S::f, s) == 3);
    }

    WHEN("its pointer to member function invoked in compile time")
    {
      constexpr S cs;
      static_assert(ezy::invoke(&S::cf, cs) == 4);
    }

    WHEN("its pointer to member function invoked on derived")
    {
      D d;
      REQUIRE(ezy::invoke(&S::f, d) == 3);
    }

    WHEN("its pointer to member invoked")
    {
      S s;
      REQUIRE(ezy::invoke(&S::mem, s) == 5);
      //REQUIRE(ezy::invoke(&S::mem, s, 1, 2, 3) == 5); // "Args cannot be provided for member pointer"
    }
  }

  GIVEN("a function")
  {
    WHEN("it invoked")
    {
      REQUIRE(ezy::invoke(foo) == 101);
    }
  }

  GIVEN("a lambda")
  {
    constexpr auto l = [] { return 102; };
    WHEN("it invoked")
    {
      REQUIRE(ezy::invoke(l) == 102);
    }
  }
}
