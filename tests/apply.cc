#include <catch2/catch.hpp>

#include <ezy/apply.h>

SCENARIO("ezy::apply is a polyfill")
{
  GIVEN("a lambda and a tuple")
  {
    auto fn = [](int a, int b, int c) { return a + b + c; };
    auto tup = std::tuple{1, 2, 3};
    WHEN("fn applied to the tuple")
    {
      auto result = ezy::apply(fn, tup);
      THEN("result contains the function result called by tuple parameters")
      {
        REQUIRE(result == 6);
      }
    }
  }

  GIVEN("a lambda which has 0 arity")
  {
    auto fn = [](){ return 10; };
    WHEN("applied to an empty tuple") {
      auto result = ezy::apply(fn, std::tuple<>{});
      THEN("result contains the function result")
      {
        REQUIRE(result == 10);
      }
    }
  }
}

SCENARIO("ezy::apply curried")
{
  GIVEN("an add lambda which adds two integers")
  {
    auto add = [](int a, int b) { return a + b; };
    WHEN("passed to unary ezy::apply")
    {
      auto apply_add = ezy::apply(add);
      THEN("apply a tuple (4, 5) returns their sum")
      {
        REQUIRE(apply_add(std::tuple{4, 5}) == 9);
      }
    }
  }

  WHEN("a temporary passed to unary ezy::apply")
  {
    auto apply_add = ezy::apply([](int a, int b) { return a + b; });
    THEN("apply a tuple (5, 6) returns their sum")
    {
      REQUIRE(apply_add(std::tuple{5, 6}) == 11);
    }
  }

  GIVEN("a stateful temporary curried to ezy::apply")
  {
    auto apply_add = ezy::apply([val = std::make_unique<int>(10)](int a, int b) {return (*val) + a + b;});
    WHEN("called with a tuple")
    {
      REQUIRE(apply_add(std::tuple{4, 3}) == 17);
    }
  }

  GIVEN("a reference to a stateful object curried to ezy::apply")
  {
    struct MyCustomAdder
    {
      int plus{};
      int operator()(int a, int b) const
      {
        return plus + a + b;
      }
    };
    auto my_add = MyCustomAdder{10};
    auto apply_add = ezy::apply(my_add);
    my_add.plus = 100;

    WHEN("called with a tuple")
    {
      const auto result = apply_add(std::tuple{4, 3});
      THEN("should use the reference (107) not the copy(17)")
      {
        REQUIRE(result == 107);
      }
    }
  }
}
