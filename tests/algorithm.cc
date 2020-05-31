#include <catch.hpp>

#include <ezy/algorithm>
#include <ezy/strong_type>
#include <ezy/features/iterable.h>
#include <ezy/string.h>

#include <vector>
#include <list>

#include "common.h"

namespace
{
  struct empty_t
  {
    bool m_is_empty{true};

    bool empty() const noexcept
    {
      return m_is_empty;
    }

    size_t size() const noexcept
    {
      return m_is_empty ? 0 : 1;
    }
  };

  struct fake_iterator
  {
    using difference_type = size_t;
    using value_type = size_t;
    using pointer = size_t;
    using reference = size_t;
    using iterator_category = std::random_access_iterator_tag;

    bool operator!=(const fake_iterator& rhs) const
    { return v != rhs.v; }

    size_t operator-(const fake_iterator& rhs) const
    { return v - rhs.v; }

    size_t v{0};
  };

  struct iterable_t
  {
    size_t m_size{0};

    fake_iterator begin() const noexcept
    {
      return {0};
    }

    fake_iterator end() const noexcept
    {
      return {m_size};
    }
  };

  struct external_iterable_t
  {
    size_t m_size{0};
  };

  fake_iterator begin(const external_iterable_t&) noexcept
  {
    return {0};
  }

  fake_iterator end(const external_iterable_t& i) noexcept
  {
    return {i.m_size};
  }

  struct external_empty_check_t
  {
    size_t v{0};
  };

  bool empty(const external_empty_check_t& t)
  {
    return t.v == 0;
  }
}

SCENARIO("empty")
{
  GIVEN("a vector")
  {
    REQUIRE(ezy::empty(std::vector<int>{}));
    REQUIRE(!ezy::empty(std::vector<int>{1}));
  }

  GIVEN("a type with member empty")
  {
    REQUIRE(ezy::empty(empty_t{true}));
    REQUIRE(!ezy::empty(empty_t{false}));
  }

  GIVEN("a type with external empty")
  {
    REQUIRE(ezy::empty(external_empty_check_t{0}));
    REQUIRE(!ezy::empty(external_empty_check_t{1}));
  }

  GIVEN("a type with begin and end")
  {
    REQUIRE(ezy::empty(iterable_t{}));
    REQUIRE(!ezy::empty(iterable_t{1}));
  }

  GIVEN("a type with external begin and end")
  {
    REQUIRE(ezy::empty(external_iterable_t{0}));
    REQUIRE(!ezy::empty(external_iterable_t{1}));
  }
}

SCENARIO("size")
{
  GIVEN("a vector")
  {
    REQUIRE(ezy::size(std::vector<int>{}) == 0);
    REQUIRE(ezy::size(std::vector<int>{1}) == 1);
  }

  GIVEN("a type with member size")
  {
    REQUIRE(ezy::size(empty_t{true}) == 0);
    REQUIRE(ezy::size(empty_t{false}) == 1);
  }

  GIVEN("a type with begin and end")
  {
    REQUIRE(ezy::size(iterable_t{}) == 0);
    REQUIRE(ezy::size(iterable_t{4}) == 4);
  }

  GIVEN("a type with external begin and end")
  {
    REQUIRE(ezy::size(external_iterable_t{}) == 0);
    REQUIRE(ezy::size(external_iterable_t{4}) == 4);
  }
}

SCENARIO("for_each")
{
  std::vector<int> v{1,2,3};
  std::string r;
  ezy::for_each(v, [&r](int i) { r += std::to_string(i); });
  REQUIRE(r == "123");
}

// however .for_each() is preferred
SCENARIO("for_each on strong type")
{
  ezy::strong_type<std::vector<int>, void, ezy::features::iterable> v{1,2,3};
  std::string r;
  ezy::for_each(v, [&r](int i) { r += std::to_string(i); });
  REQUIRE(r == "123");
}

SCENARIO("join empty range")
{
  std::vector<std::string> v{};
  const auto joined = ezy::join(v, "+");
  REQUIRE(joined == "");
}

SCENARIO("join")
{
  std::vector<std::string> v{"a", "b", "c"};
  REQUIRE(ezy::join(v, "+") == "a+b+c");
}

SCENARIO("join with default delimiter")
{
  std::vector<std::string> v{"a", "b", "c"};
  REQUIRE(ezy::join(v) == "abc");
}

SCENARIO("join string_views")
{
  std::vector<std::string_view> v{"a", "b", "c"};
  REQUIRE(ezy::join<std::string>(v, ":") == "a:b:c");
}

template <typename Range>
std::string
join_as_strings(Range&& range, std::string_view separator = "")
{
  return ezy::join(ezy::transform(std::forward<Range>(range), ezy::to_string), separator);
}

SCENARIO("transform")
{
  std::vector<int> v{1,2,3};
  std::string r;
  auto mapped = ezy::transform(v, [](int i) { return i + 1; });
  REQUIRE(join_as_strings(mapped) == "234");
}

SCENARIO("concatenate")
{
  std::vector<int> v1{1,2,3};
  std::vector<int> v2{4,5,6};
  const auto concatenated = ezy::concatenate(v1, v2);
  REQUIRE(join_as_strings(concatenated) == "123456");
}

SCENARIO("concatenate - when the first is a temporary")
{
  std::vector<int> v2{4,5,6};
  const auto concatenated = ezy::concatenate(std::vector{1,2,3}, v2);
  REQUIRE(join_as_strings(concatenated) == "123456");
}

SCENARIO("concatenate - when the second is a temporary")
{
  std::vector<int> v1{1,2,3};
  const auto concatenated = ezy::concatenate(v1, std::vector{4,5,6});
  REQUIRE(join_as_strings(concatenated) == "123456");
}

SCENARIO("concatenate - when the both are temporaries")
{
  const auto concatenated = ezy::concatenate(std::vector{1,2,3}, std::vector{4,5,6});
  REQUIRE(join_as_strings(concatenated) == "123456");
}

template <typename Zipped>
auto join_zipped(Zipped&& zipped)
{
  return ezy::join(
      ezy::transform(
        zipped,
        [](auto pair) -> std::string {auto[a,b] = pair; return std::to_string(a) + "+" + std::to_string(b) + ";"; }
  ));
}

SCENARIO("zip")
{
  std::vector<int> v1{1,2,3};
  std::vector<int> v2{4,5,6};
  const auto zipped = ezy::zip(v1, v2);
  const auto joined = join_zipped(zipped);
  REQUIRE(joined == "1+4;2+5;3+6;");

  THEN("value type is what expected")
  {
    using Zipped = std::remove_const_t<decltype(zipped)>;
    static_assert(std::is_same_v<typename Zipped::const_iterator::value_type, std::tuple<int, int>>);
    static_assert(std::is_same_v<typename Zipped::value_type, std::tuple<int, int>>);
  }
}

SCENARIO("zip temporaries")
{
  const auto zipped = ezy::zip(std::vector{1,2,3}, std::vector{4,5,6});
  const auto joined = join_zipped(zipped);
  REQUIRE(joined == "1+4;2+5;3+6;");
}

SCENARIO("zip different size")
{
  const auto zipped = ezy::zip(std::vector{1,2,3}, std::vector{4, 5});
  const auto joined = join_zipped(zipped);
  REQUIRE(joined == "1+4;2+5;");
}

SCENARIO("zip 3")
{
  const auto zipped = ezy::zip(std::vector{1,2,3}, std::vector{4,5,6}, std::vector{7,8,9});
  const auto joined = ezy::join(
      ezy::transform(
        zipped,
        [](auto tuple) -> std::string {auto[a,b,c] = tuple; return std::to_string(a) + "+" + std::to_string(b) + "+" + std::to_string(c) + ";"; }
  ));
  REQUIRE(joined == "1+4+7;2+5+8;3+6+9;");

  THEN("value type is expected")
  {
    using Zipped = std::remove_const_t<decltype(zipped)>;
    static_assert(std::is_same_v<typename Zipped::value_type, std::tuple<int, int, int>>);
  }
}

SCENARIO("mapped and zipped")
{
  const auto zipped = ezy::zip(std::vector{1,2,3}, ezy::transform(std::vector{1,2,3}, [](int i) { return i + 1; }));
  const auto joined = join_zipped(zipped);
  REQUIRE(joined == "1+2;2+3;3+4;");

  THEN("value type is expected")
  {
    using Zipped = std::remove_const_t<decltype(zipped)>;
    static_assert(std::is_same_v<typename Zipped::value_type, std::tuple<int, int>>);
  }
}

SCENARIO("zip_with")
{
  const auto zipped = ezy::zip_with(std::plus{}, std::vector{1,2,3}, std::vector{4,5,6});
  const auto joined = join_as_strings(zipped, ",");
  REQUIRE(joined == "5,7,9");

  THEN("zipper function size can be optimized out from the iterator")
  {
    using ZipIterator = decltype(std::begin(zipped));
    static_assert(sizeof(ZipIterator) == (sizeof(ezy::detail::iterator_tracker_for<std::vector<int>>) * 2));
  }

  THEN("value type is expected")
  {
    using Zipped = std::remove_const_t<decltype(zipped)>;
    static_assert(std::is_same_v<typename Zipped::value_type, int>);
  }
}

SCENARIO("slice")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  REQUIRE(join_as_strings(ezy::slice(v, 0, 3)) == "123");
}

SCENARIO("take")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  REQUIRE(join_as_strings(ezy::take(v, 5)) == "12345");
}

SCENARIO("take_while")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  REQUIRE(join_as_strings(ezy::take_while(v, [](int i) { return i != 5; })) == "1234");
}

SCENARIO("take_while temporary")
{
  REQUIRE(join_as_strings(ezy::take_while(
          std::vector{1,2,3,4,5,6,7,8},
          [](int i) { return i != 5; })) == "1234");
}

SCENARIO("flatten")
{
  std::vector<std::vector<int>> v{std::vector{1,2,3,4,5,6,7,8}, {}, std::vector{0,0,0}};
  REQUIRE(join_as_strings(ezy::flatten(v)) == "12345678000");
}

SCENARIO("find_element")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  const auto found = ezy::find_element(v, 5);
  REQUIRE(found != std::end(v));
  REQUIRE(*found == 5);

  const auto not_found = ezy::find_element(v, 9);
  REQUIRE(not_found == std::end(v));
}

SCENARIO("find_element in temporary should not compile")
{
  //const auto found = ezy::find_element(std::vector{1,2,3,4,5,6,7,8}, 5);
  // > "Range must be a reference! Cannot form an iterator to a temporary!"
}

SCENARIO("find")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  const auto found = ezy::find(v, 5);
  REQUIRE(found.has_value());
  REQUIRE(found.value() == 5); // this should be a reference

  const auto not_found = ezy::find(v, 9);
  REQUIRE(!not_found.has_value());
}

SCENARIO("find returns reference-like")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  const auto found = ezy::find(v, 5);
  REQUIRE(found.has_value());
  REQUIRE(found.value() == 5);
  v[4] += 10;
  REQUIRE(found.value() == 15);
}

SCENARIO("find in temporary")
{
  const auto found = ezy::find(std::vector{1,2,3,4,5,6,7,8}, 6);
  REQUIRE(found.has_value());
  REQUIRE(found.value() == 6); // this must be moved
}

constexpr auto greater_than_3 = [](int i) { return i > 3; };
constexpr auto greater_than_10 = [](int i) { return i > 10; };

SCENARIO("find_if")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  const auto found = ezy::find_if(v, greater_than_3);
  REQUIRE(found.has_value());
  REQUIRE(found.value() == 4);

  const auto not_found = ezy::find_if(v, greater_than_10);
  REQUIRE(!not_found.has_value());
}

SCENARIO("find_if in temporary")
{
  const auto found = ezy::find_if(std::vector{1,2,3,4,5,6,7,8}, greater_than_3);
  REQUIRE(found.has_value());
  REQUIRE(found.value() == 4);

  const auto not_found = ezy::find_if(std::vector{1,2,3,4,5,6,7,8}, greater_than_10);
  REQUIRE(!not_found.has_value());
}

SCENARIO("find_element_if")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  const auto found = ezy::find_element_if(v, greater_than_3);
  REQUIRE(found != end(v));
  REQUIRE(*found == 4);

  const auto not_found = ezy::find_element_if(v, greater_than_10);
  REQUIRE(not_found == end(v));
}

SCENARIO("find_element_if in temporary should not compile")
{
  //const auto found = ezy::find_element_if(std::vector{1,2,3,4,5,6,7,8}, greater_than_3);
  // > "Range must be a reference! Cannot form an iterator to a temporary!"
}

bool operator==(const move_only& lhs, const move_only& rhs)
{
  return lhs.i == rhs.i;
}

// TODO extract
template <typename T>
struct ctor_of
{
  using value_type = T;
  template <typename... Args>
  constexpr value_type operator()(Args&&... args) const noexcept(noexcept(value_type(std::forward<Args>(args)...)))
  {
    return value_type(std::forward<Args>(args)...);
  }
};

constexpr auto make_vector_of_move_only = [] {
  return ezy::collect<std::vector>(ezy::transform(std::vector{1,2,3,4,5}, ctor_of<move_only>{}));
};

SCENARIO("find a move_only type")
{
  auto found = ezy::find(make_vector_of_move_only(), move_only{3});
  REQUIRE(found.has_value());
  REQUIRE(found->i == 3);
}

SCENARIO("find_if a move_only type")
{
  auto found = ezy::find_if(make_vector_of_move_only(), [](const auto& m) { return m.i == 4; });
  REQUIRE(found.has_value());
  REQUIRE(found->i == 4);
}

SCENARIO("collect explicitly to the same type")
{
  std::vector<int> v{1,2,3,4};
  auto collected = ezy::collect<std::vector<int>>(v);
  static_assert(std::is_same_v<decltype(v), decltype(collected)>);
  REQUIRE(v == collected);
}

SCENARIO("collect explicitly to different type")
{
  std::vector<int> v{1,2,3,4};
  auto collected = ezy::collect<std::vector<unsigned int>>(v);
  REQUIRE(collected == std::vector{1u,2u,3u,4u});
}

SCENARIO("collect explicitly to different container type")
{
  std::vector<int> v{1,2,3,4};
  auto collected = ezy::collect<std::list<int>>(v);
  REQUIRE(collected == std::list{1,2,3,4});
}

SCENARIO("collect by deducing the element type")
{
  std::vector<int> v{1,2,3,4};
  auto collected = ezy::collect<std::vector>(v);
  static_assert(std::is_same_v<decltype(collected), std::vector<int>>);
  REQUIRE(collected == v);
}

SCENARIO("collect const by deducing the element type")
{
  const std::vector<int> v{1,2,3,4};
  auto collected = ezy::collect<std::vector>(v);
  static_assert(std::is_same_v<decltype(collected), std::vector<int>>);
  REQUIRE(collected == v);
}

SCENARIO("iterate")
{
  const auto it = ezy::iterate(1, [](int i) { return i * 2;});
  const auto joined = join_as_strings(ezy::take(it, 7), ",");
  REQUIRE(joined == "1,2,4,8,16,32,64");
}

SCENARIO("enumerate")
{
  std::vector<std::string> fruits{"alma", "banan", "cseresznye"};
  const auto enumerated = ezy::enumerate(fruits);
  const auto joined = ezy::join(
      ezy::transform(
        enumerated,
        [](const auto& pair) { const auto& [i, f] = pair; return std::to_string(i) + "+" + f; }
        ),
      ";");
  REQUIRE(joined == "0+alma;1+banan;2+cseresznye");
}
