#include <catch2/catch.hpp>

#include <ezy/features/nullable.h>
#include <ezy/features/nullable_result.h>
#include <ezy/experimental/value_provider.h>

SCENARIO("default_nullable")
{
  using Number = ezy::strong_type<int, void, ezy::features::experimental::default_nullable>;
  GIVEN("a default constructed instance")
  {
    Number num{};
    WHEN("query a default constructed element")
    {
      REQUIRE(!num.has_value());
      REQUIRE(num.value() == 0);
    }

    WHEN("dereferenced")
    {
      THEN("returns the underlying value")
      {
        REQUIRE(*num == 0);
      }

      THEN("it can be mutated")
      {
        *num += 10;
        REQUIRE(*num == 10);
        REQUIRE(num.has_value());
      }
    }

    WHEN("converted to bool")
    {
      THEN("false")
      {
        REQUIRE(!num);
      }
    }
  }

  GIVEN("a non-null instance")
  {
    Number num{3};
    WHEN("query a non-null element")
    {
      REQUIRE(num.has_value());
      REQUIRE(num.value() == 3);
    }

    WHEN("dereferenced")
    {
      THEN("returns the underlying value")
      {
        REQUIRE(*num == 3);
      }

      THEN("it can be mutated")
      {
        *num -= 3;
        REQUIRE(*num == 0);
        REQUIRE(!num.has_value());
      }
    }

    WHEN("converted to bool")
    {
      THEN("returns true")
      {
        REQUIRE((num ? true : false));
      }
    }
  }
}

constexpr int return_10() noexcept
{
  return 10;
}

constexpr bool is_zero(int i) noexcept
{
  return i == 0;
}

bool is_empty(const std::string& s)
{
  return s.empty();
}

SCENARIO("nullable_as")
{
  using Number = ezy::strong_type<int, void,
        ezy::features::experimental::nullable_as<ezy::experimental::value_provider<-1>>
  >;

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
    static_assert(Number::make_null().value() == -1);
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
        ezy::features::experimental::nullable_as<ezy::experimental::value_provider<5>, is_divisible_fn>>;

  static_assert(Number::is_consistent());
  REQUIRE(Number::is_consistent());
  REQUIRE(Number(1).has_value());
  REQUIRE(Number(2).has_value());
  REQUIRE(Number(3).has_value());
  REQUIRE(!Number(10).has_value());
  REQUIRE(Number(11).has_value());
}

constexpr bool is_even(int i) noexcept
{
  return (i % 2) == 0;
}

SCENARIO("nullable_if")
{

  struct is_negative_fn
  {
    constexpr int operator()(int i) noexcept
    {
      return (i < 0);
    }
  };

  using Number = ezy::strong_type<int, void, ezy::features::experimental::nullable_if<is_negative_fn>>;
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

// These cases are not allowed to be compiled, they might result in crash or hard to reason compiler error message.
// Consider using function wrapper from tutorials
SCENARIO("do not compile")
{
  GIVEN("nullable with an ordinary function")
  {
    //using Number = ezy::strong_type<int, void, ezy::features::experimental::nullable_as<decltype(return_10)>>;
    //Number n{};
    //n.has_value();
    //using Number2 = ezy::strong_type<int, void,
    //    ezy::features::experimental::nullable_as<decltype(return_10), decltype(is_zero)>
    //>;
    //(void)Number2{};
  }

  GIVEN("a string with a member function should not compile")
  {

    //using N = ezy::strong_type<int, void, ezy::features::experimental::nullable_if<decltype(is_zero)>>;
    //(void)N{};
    //using Str = ezy::strong_type<std::string, void,
    //    ezy::features::experimental::nullable_if<decltype(is_empty)>
    //>;
    //(void)Str{};
    //using Str = ezy::strong_type<std::string, void,
    //    ezy::features::experimental::nullable_if<decltype(&std::string::empty)>
    //>;
    //(void)Str{};
  }

  GIVEN("a nullable type with an function type should not compile")
  {
    //using NumberNotEven = ezy::strong_type<int, void,
    //    ezy::features::experimental::nullable_if<decltype(is_even)>
    //>;
    //(void)NumberNotEven{};
  }

  GIVEN("a nullable type with a lambda type does not compile")
  {
    // constexpr auto is_odd = [](int i)
    // {
    //   return !is_even(i);
    // };
    // It could if stateless lambda was default constructible.
    //using NumberNotOdd = ezy::strong_type<int, void,
    //    ezy::features::experimental::nullable_if<decltype(is_odd)>
    //>;
    //(void)NumberNotOdd{};
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

#include <ezy/result.h>

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
            ezy::features::experimental::nullable_as<ezy::experimental::value_provider<5>>>;

      const auto mapped = ptr.map<Number>(plus_one_and_multiply);
      REQUIRE(!mapped.has_value());
      REQUIRE(mapped.value() == 5);
    }
    */
  }

}


SCENARIO("ezy::pointer accessors")
{
  struct S
  {
    int i{10};
    std::string name{"asd"};

    bool operator==(const S& rhs) const {
      return (i == rhs.i) && (name == rhs.name);
    }
  };

  GIVEN("a pointer to a struct")
  {
    S s{12, "Name"};
    Pointer<S> ptr{&s};
    THEN("it can be dereferenced")
    {
      REQUIRE(*ptr == s);
    }

    THEN("its members can be accessed with '->'")
    {
      REQUIRE(ptr->i == 12);
      REQUIRE(ptr->name == "Name");
    }

    WHEN("its member modified")
    {
      ptr->i += 10;
      THEN("it is truly changed")
      {
        REQUIRE(s.i == 22);
      }
    }
  }

  GIVEN("a const pointer to a struct")
  {
    S s{12, "Name"};
    Pointer<const S> ptr{&s};
    THEN("it can be dereferenced")
    {
      static_assert(std::is_same<decltype(*ptr), const S&>::value);
      REQUIRE(*ptr == s);
    }

    THEN("its members can be accessed with '->'")
    {
      static_assert(std::is_same<decltype((ptr->i)), const int&>::value);
      static_assert(std::is_same<decltype((ptr->name)), const std::string&>::value);
      REQUIRE(ptr->i == 12);
      REQUIRE(ptr->name == "Name");
    }
  }
}
