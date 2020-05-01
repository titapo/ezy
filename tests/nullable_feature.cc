#include <catch.hpp>

#include <ezy/features/nullable.h>
#include <ezy/features/nullable_result.h>

SCENARIO("default_nullable")
{
  using Number = ezy::strong_type<int, void, ezy::features::experimental::default_nullable>;
  GIVEN("a default constructed instance")
  {
    WHEN("query a default constructed element")
    {
      Number num{};
      REQUIRE(!num.has_value());
      REQUIRE(num.value() == 0);
    }
  }

  GIVEN("a non-null instance")
  {
    WHEN("query a non-null element")
    {
      Number num{3};
      REQUIRE(num.has_value());
      REQUIRE(num.value() == 3);
    }
  }
}

SCENARIO("basic_nullable_as")
{
  using Number = ezy::strong_type<int, void, ezy::features::experimental::basic_nullable_as<ezy::experimental::value_provider<-1>>::apply>;

  GIVEN("a default constructed instance")
  {
    THEN("not a null")
    {
      Number num{};
      REQUIRE(num.has_value());
      REQUIRE(num.value() == 0);
    }
  }

  GIVEN("a default constructed instance")
  {
    THEN("not a null")
    {
      Number num{};
      REQUIRE(num.has_value());
      REQUIRE(num.value() == 0);
    }
  }

  GIVEN("instance holding -1")
  {
    THEN("is a null")
    {
      Number num{-1};
      REQUIRE(!num.has_value());
      REQUIRE(num.value() == -1);
    }
  }

  GIVEN("static make_null member")
  {
    static_assert(Number::make_null() == -1);
  }
  // TODO value_provider::value_type and underlying type has to match
}

SCENARIO("basic nullable condition can be specified")
{
  struct is_divisible_fn
  {
    constexpr int operator()(int i, int by) noexcept
    {
      return (i % by) == 0;
    }
  };

  using Number = ezy::strong_type<int, void,
        ezy::features::experimental::basic_nullable_as<ezy::experimental::value_provider<5>, is_divisible_fn>::apply>;

  static_assert(Number::is_consistent());
  REQUIRE(Number::is_consistent());
  REQUIRE(Number(1).has_value());
  REQUIRE(Number(2).has_value());
  REQUIRE(Number(3).has_value());
  REQUIRE(!Number(10).has_value());
  REQUIRE(Number(11).has_value());
}


SCENARIO("basic_nullable_if")
{

  struct is_negative_fn
  {
    constexpr int operator()(int i) noexcept
    {
      return (i < 0);
    }
  };

  using Number = ezy::strong_type<int, void, ezy::features::experimental::nullable_if<is_negative_fn>::apply>;
  GIVEN("positive number")
  {
    Number positive{20};
    REQUIRE(positive.has_value());
  }

  GIVEN("zero")
  {
    Number zero{0};
    REQUIRE(zero.has_value());
  }

  GIVEN("negative")
  {
    Number negative{-1};
    REQUIRE(!negative.has_value());
  }
}

template <typename T, typename Tag = void>
using Result = ezy::strong_type<T, Tag, ezy::features::experimental::nullable>;

SCENARIO("nullable result")
{
  using IntResult = Result<int, struct IntTag>;

  GIVEN("a non-null")
  {
    constexpr auto plus_one_and_multiply = [](int i) { return (i + 1) * 2;};
    constexpr auto add_half = [](int i) -> double { return i + 0.5;};

    const IntResult nr{3};
    WHEN("has_value checked")
    {
      REQUIRE(nr.has_value());
    }

    WHEN("mapped (closed)")
    {
      const auto mapped = nr.map(plus_one_and_multiply);
      static_assert(std::is_same_v<decltype(mapped), const IntResult>);
      REQUIRE(mapped.has_value());
      REQUIRE(mapped.value() == 8);
    }

    WHEN("mapped (opened)")
    {
      const auto mapped = nr.map(add_half);
      static_assert(std::is_same_v<decltype(mapped), const Result<double, IntTag>>); // tag is preserved
      REQUIRE(mapped.has_value());
      REQUIRE(mapped.value() == 3.5);
    }

    WHEN("mapped with concrete return type")
    {
      const auto mapped = nr.map<Result<double, struct DoubleTag>>(add_half);
      static_assert(std::is_same_v<decltype(mapped), const Result<double, DoubleTag>>);
      REQUIRE(mapped.has_value());
      REQUIRE(mapped.value() == 3.5);
    }

    WHEN("mapped with concrete return type (cast)")
    {
      // const auto mapped = nr.map<IntResult>(add_half); // OK: does not compile
    }
  }
}

template <typename T, typename Tag = void>
using Pointer = ezy::strong_type<T*, Tag, ezy::features::experimental::nullable_ptr>;

#include <ezy/result>

SCENARIO("pointer")
{
  constexpr auto plus_one_and_multiply = [](int i) { return (i + 1) * 2;};
  constexpr auto add_half = [](int i) -> double { return i + 0.5;};
  constexpr auto to_null_str = [](int*){ return "null";};

  int i = 10;
  GIVEN("a pointer")
  {
    Pointer<int> ptr{&i};
    WHEN("mapped")
    {
      const auto mapped = ptr.map<Result<int>>(plus_one_and_multiply).map(add_half);
      REQUIRE(mapped.has_value());
      REQUIRE(mapped.value() == 22.5);
    }

    WHEN("map_error and map")
    {
      const auto mapped = ptr.map_error<ezy::result<int, std::string>>(to_null_str).map(add_half);
      REQUIRE(mapped.is_success());
      REQUIRE(std::get<double>(mapped.get()) == 10.5);
    }
  }

  GIVEN("a null pointer")
  {
    Pointer<int> ptr{nullptr};
    REQUIRE(!ptr.has_value());

    WHEN("map_error and map")
    {
      const auto mapped = ptr.map_error<ezy::result<int, std::string>>(to_null_str).map(add_half);
      REQUIRE(mapped.is_error());
      REQUIRE(std::get<std::string>(mapped.get()) == "null");
    }

    /*
     * TODO clarify
    WHEN("mapped to a type whose null is not a default")
    {
      using Number = ezy::strong_type<int, void,
            ezy::features::experimental::basic_nullable_as<ezy::experimental::value_provider<5>>::apply>;

      const auto mapped = ptr.map<Number>(plus_one_and_multiply);
      REQUIRE(!mapped.has_value());
      REQUIRE(mapped.value() == 5);
    }
    */
  }
}
