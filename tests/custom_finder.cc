#include <catch.hpp>
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
      THEN("it returns an not engaded")
      {
        REQUIRE(!found.engaged);
      }
    }
  }
}

#include <ezy/experimental/function.h>

SCENARIO("custom find if")
{
  static constexpr auto myfind_if = ezy::custom_find_if<fake_result_maker>{};
  GIVEN("a range of numbers")
  {
    std::vector v{1,2,3,4,5};
    const auto gt = ezy::experimental::curry(ezy::experimental::flip(std::greater<>{}));

    WHEN("find for an element")
    {
      auto found = myfind_if(v, gt(2));
      THEN("it returns an engaged, containing 3")
      {
        REQUIRE(found.engaged);
        REQUIRE(found.value == 3);
      }
    }

    WHEN("find for a non-presenting element")
    {
      auto found = myfind_if(v, gt(5));
      THEN("it returns an not engaded")
      {
        REQUIRE(!found.engaged);
      }
    }
  }

}
