#include <catch2/catch.hpp>
#include <vector>
#include <ezy/custom_find.h>

template <typename T>
struct fake_optional
{
  bool engaged{false};
  T value{};
};

struct fake_result_maker
{
  template <typename Range, typename Iterator>
  auto operator()(Iterator it, Iterator last)
  {
    using T = std::remove_reference_t<decltype(*begin(std::declval<Range>()))>;
    if (it != last)
      return fake_optional<T>{true, *it};
    else
      return fake_optional<T>{false, {}};
  }
};

SCENARIO("custom find")
{
  static constexpr auto myfind = ezy::custom_find<fake_result_maker>{};
  GIVEN("a range of numbers")
  {
    std::vector v{1,2,3,4,5};

    WHEN("find for an element")
    {
      auto found = myfind(v, 3);
      THEN("it returns an engaged, containing 3")
      {
        REQUIRE(found.engaged);
        REQUIRE(found.value == 3);
      }
    }

    WHEN("find for a non-presenting element")
    {
      auto found = myfind(v, 8);
      THEN("it returns an not engaged")
      {
        REQUIRE(!found.engaged);
      }
    }
  }
}

#include <ezy/experimental/function.h>

const auto greater_than = ezy::experimental::curry(ezy::experimental::flip(std::greater<>{}));

SCENARIO("custom find if")
{
  static constexpr auto myfind_if = ezy::custom_find_if<fake_result_maker>{};
  GIVEN("a range of numbers")
  {
    std::vector v{1,2,3,4,5};

    WHEN("find for an element")
    {
      auto found = myfind_if(v, greater_than(2));
      THEN("it returns an engaged, containing 3")
      {
        REQUIRE(found.engaged);
        REQUIRE(found.value == 3);
      }
    }

    WHEN("find for a non-presenting element")
    {
      auto found = myfind_if(v, greater_than(5));
      THEN("it returns an not engaged")
      {
        REQUIRE(!found.engaged);
      }
    }
  }
}

#include <ezy/features/algo_custom_find.h>

using my_find_feature = ezy::features::algo_custom_find<fake_result_maker>;

#include <ezy/strong_type.h>

SCENARIO("custom find feature")
{
  using Strong = ezy::strong_type<std::vector<int>, struct Tag, my_find_feature>;

  Strong st{1,2,3,4};

  WHEN("find an exising element")
  {
    const auto found = st.find(3);
    THEN("returns a fake_optional")
    {
      REQUIRE(found.engaged);
      REQUIRE(found.value == 3);
    }
  }

  WHEN("find an non-exising element")
  {
    const auto found = st.find(6);
    THEN("returns a not-engaged fake_optional")
    {
      REQUIRE(!found.engaged);
    }
  }

  WHEN("find_if an exising element")
  {
    const auto found = st.find_if(greater_than(3));
    THEN("returns a fake_optional")
    {
      REQUIRE(found.engaged);
      REQUIRE(found.value == 4);
    }
  }

  WHEN("find_if an non-exising element")
  {
    const auto found = st.find_if(greater_than(4));
    THEN("returns a not-engaged fake_optional")
    {
      REQUIRE(!found.engaged);
    }
  }
}
