#include <catch2/catch.hpp>

#include <ezy/algorithm>
#include <ezy/strong_type>
#include <ezy/features/iterable.h>
#include <ezy/string.h>
#include <ezy/experimental/function.h>

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

  struct external_size_check_t
  {
    size_t v{0};
  };

  size_t size(const external_size_check_t& t)
  {
    return t.v;
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

  GIVEN("a type with external size")
  {
    REQUIRE(ezy::size(external_size_check_t{0}) == 0);
    REQUIRE(ezy::size(external_size_check_t{3}) == 3);
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

  GIVEN("a c-style array")
  {
    const int a[] = {1, 4, 5};
    REQUIRE(ezy::size(a) == 3);
    static_assert(std::is_same<decltype(ezy::size(a)), std::size_t>::value);
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

SCENARIO("for_each on array")
{
  int a[] = {2, 3, 4};
  std::string r;
  ezy::for_each(a, [&](int i) { r += std::to_string(i); });
  REQUIRE(r == "234");
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

SCENARIO("filter")
{
  std::vector<int> v{1,2,3,4,5,6};
  auto filtered = ezy::filter(v, [](int i) { return i % 2 == 0; });
  REQUIRE(join_as_strings(filtered) == "246");
}

SCENARIO("filter array")
{
  const int a[] = {1,2,3,4,5,6};
  auto filtered = ezy::filter(a, [](int i) { return i % 2 == 0; });
  REQUIRE(join_as_strings(filtered) == "246");
}

SCENARIO("filter mutable")
{
  std::vector<int> v{1,2,3,4,5,6};
  auto filtered = ezy::filter(v, [](int i) { return i % 2 == 0; });
  for (auto& i : filtered)
  {
    i *= 2;
  };
  REQUIRE(join_as_strings(filtered, ",") == "4,8,12");
  REQUIRE(join_as_strings(v, ",") == "1,4,3,8,5,12");
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

SCENARIO("concatenate - on array")
{
  int a1[] = {1, 2, 3};
  int a2[] = {4, 5, 6};
  const auto concatenated = ezy::concatenate(a1, a2);
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
    // TODO assertions against for zip_forward(?) value_types
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

// consider a builtin algorithm (eg. zip_forward)
SCENARIO("zip mutable reference")
{
  std::vector ref{1, 2, 3, 4, 5};
  const std::vector addition{10, 20, 30, 40, 50};

  for (const auto [r, add] : ezy::zip_with(ezy::forward_as_tuple, ref, addition))
  {
    r += add;
  }

  const auto joined = ezy::join(ezy::transform(ref, ezy::to_string), ",");
  REQUIRE(joined == "11,22,33,44,55");
}

SCENARIO("zip rvalue ")
{
  std::vector ref{1, 2, 3, 4, 5};

  for (const auto [r, add] : ezy::zip_with(ezy::forward_as_tuple, ref, std::vector{10, 20, 30, 40, 50}))
  {
    r += add;
  }

  const auto joined = ezy::join(ezy::transform(ref, ezy::to_string), ",");
  REQUIRE(joined == "11,22,33,44,55");
}

SCENARIO("slice")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  REQUIRE(join_as_strings(ezy::slice(v, 0, 3)) == "123");
}

SCENARIO("slice on array")
{
  int a[] = {1,2,3,4,5,6,7,8};
  REQUIRE(join_as_strings(ezy::slice(a, 2, 5)) == "345");
}

static constexpr auto greater_than = ezy::experimental::curry(ezy::experimental::flip(std::greater<>{}));
static constexpr auto less_than = ezy::experimental::curry(ezy::experimental::flip(std::less<>{}));

SCENARIO("all_of")
{
  int a[] = {0, 1, 2, 3, 4, 5};
  REQUIRE(ezy::all_of(a, greater_than(-1)));
  REQUIRE(!ezy::all_of(a, greater_than(0)));
}

SCENARIO("any_of")
{
  int a[] = {0, 1, 2, 3, 4, 5};
  REQUIRE(ezy::any_of(a, greater_than(-1)));
  REQUIRE(ezy::any_of(a, less_than(1)));
  REQUIRE(!ezy::any_of(a, less_than(0)));
}

SCENARIO("none_of")
{
  int a[] = {0, 1, 2, 3, 4, 5};
  REQUIRE(ezy::none_of(a, greater_than(5)));
  REQUIRE(!ezy::none_of(a, less_than(1)));
  REQUIRE(ezy::none_of(a, less_than(0)));
}

SCENARIO("take")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  REQUIRE(join_as_strings(ezy::take(v, 5)) == "12345");
}

SCENARIO("take longer than element size")
{
  std::vector<int> v{1,2,3};
  REQUIRE(join_as_strings(ezy::take(v, 5), ",") == "1,2,3");
}

SCENARIO("take allows mutating")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  auto taken = ezy::take(v, 5);
  (*std::next(std::begin(taken), 2)) += 4;
  REQUIRE(join_as_strings(taken) == "12745");
  REQUIRE(join_as_strings(v) == "12745678");
}

SCENARIO("take works on array")
{
  int a[] = {1,2,3,4,5,6,7,8};
  REQUIRE(join_as_strings(ezy::take(a, 4)) == "1234");
}

SCENARIO("take works in compile time")
{
  constexpr auto taken = ezy::take(std::array{1,2,3,4,5}, 3);
  static_assert(ezy::accumulate(taken, 0) == 6);
}

SCENARIO("take_while")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  REQUIRE(join_as_strings(ezy::take_while(v, [](int i) { return i != 5; })) == "1234");
}

SCENARIO("take_while allows mutating")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  auto taken = ezy::take_while(v, [](int i) { return i != 5; });
  (*std::next(std::begin(taken), 3)) += 4;
  REQUIRE(join_as_strings(taken) == "1238");
  REQUIRE(join_as_strings(v) == "12385678");
}

SCENARIO("take_while temporary")
{
  REQUIRE(join_as_strings(ezy::take_while(
          std::vector{1,2,3,4,5,6,7,8},
          [](int i) { return i != 5; })) == "1234");
}

SCENARIO("take_while takes the whole range")
{
  THEN("it should not overrun")
  {
    std::vector<int> v{1,2,3,4,5,6,7,8};
    REQUIRE(join_as_strings(ezy::take_while(v, [](int i) { return i != 0; })) == "12345678");
  }
}

SCENARIO("zipped and taken")
{
  WHEN("both are finite")
  {
    auto result = ezy::take(ezy::zip(std::vector{1,2,3,4,5,6}, std::vector{-1,-2,-3,-4}), 8);
    REQUIRE(join_zipped(result) == "1+-1;2+-2;3+-3;4+-4;");
  }

  WHEN("one is infinite")
  {
    auto result = ezy::take(
        ezy::zip(
          ezy::iterate(1),
          std::vector{-1,-2,-3,-4}
        ),
        8);

    REQUIRE(join_zipped(result) == "1+-1;2+-2;3+-3;4+-4;");
  }

  WHEN("both are infinite")
  {
    auto result = ezy::take(
        ezy::zip(
          ezy::iterate(1, [](int i){return i + 1; }),
          ezy::iterate(-1, [](int i){return i - 1; })
        ),
        8);

    REQUIRE(join_zipped(result) == "1+-1;2+-2;3+-3;4+-4;5+-5;6+-6;7+-7;8+-8;");
  }
}

SCENARIO("drop")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  REQUIRE(join_as_strings(ezy::drop(v, 3)) == "45678");
}

SCENARIO("drop: mutate element in remaining range")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  auto remaining = ezy::drop(v, 5);
  *(remaining.begin()) -= 3;
  REQUIRE(join_as_strings(remaining) == "378");
}

SCENARIO("drop from rvalue")
{
  REQUIRE(join_as_strings(ezy::drop(std::vector{1,2,3,4,5}, 2)) == "345");
}

SCENARIO("drop from rvalue and it mutating")
{
  /* It works because `remaining` stores the original range by value*/
  auto remaining = ezy::drop(std::vector{1,2,3,4,5}, 2);
  *(remaining.begin()) -= 3;
  REQUIRE(join_as_strings(remaining) == "045");
}

SCENARIO("drop from lazy range and it mutating")
{
  auto remaining = ezy::drop(ezy::iterate(1), 2);
  *(remaining.begin()) -= 3; // modifies a temporary, it has no effect
  REQUIRE(join_as_strings(ezy::take(remaining, 3)) == "345");
}

SCENARIO("drop works on array")
{
  int a[] = {1,2,3,4,5,6,7,8};
  REQUIRE(join_as_strings(ezy::drop(a, 4)) == "5678");
}

SCENARIO("step_by")
{
  auto remaining = ezy::step_by(ezy::iterate(0), 3);
  REQUIRE(join_as_strings(ezy::take(remaining, 5), ",") == "0,3,6,9,12");
}

SCENARIO("step_by mutating")
{
  std::vector<int> v{1,2,3,4,5,6,7};
  auto remaining = ezy::step_by(v, 3);
  *(std::next(remaining.begin())) += 10;
  REQUIRE(join_as_strings(remaining, ",") == "1,14,7");
}

SCENARIO("step_by works on array")
{
  int a[] = {1,2,3,4,5,6,7,8};
  REQUIRE(join_as_strings(ezy::step_by(a, 3), ",") == "1,4,7");
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

SCENARIO("find returns mutable reference-like")
{
  std::vector<int> v{1,2,3,4,5,6,7,8};
  const auto found = ezy::find(v, 5);
  REQUIRE(found.has_value());
  REQUIRE(found.value() == 5);
  found.value() += 20;
  REQUIRE(v[4] == 25);
}

SCENARIO("find in temporary")
{
  const auto found = ezy::find(std::vector{1,2,3,4,5,6,7,8}, 6);
  REQUIRE(found.has_value());
  REQUIRE(found.value() == 6); // this must be moved
}

SCENARIO("find in temporary is mutable")
{
  auto found = ezy::find(std::vector{1,2,3,4,5,6,7,8}, 6);
  REQUIRE(found.has_value());
  REQUIRE(found.value() == 6);
  found.value() += 4;
  REQUIRE(found.value() == 10);
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

SCENARIO("find_if in temporary returns mutable")
{
  auto found = ezy::find_if(std::vector{1,2,3,4,5,6,7,8}, greater_than_3);
  REQUIRE(found.has_value());
  REQUIRE(found.value() == 4);
  found.value() += 3;
  REQUIRE(found.value() == 7);
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

SCENARIO("contains")
{
  std::vector<int> v{1,2,3};
  REQUIRE(ezy::contains(v, 0) == false);
  REQUIRE(ezy::contains(v, 1) == true);
  REQUIRE(ezy::contains(v, 2) == true);
  REQUIRE(ezy::contains(v, 3) == true);
  REQUIRE(ezy::contains(v, 4) == false);

  // does not work: std::find cannot be used in constexpr context
  //static_assert(ezy::contains(std::array{1, 5, 7}, 5) == true);
  //static_assert(ezy::contains(std::array{1, 5, 7}, 4) == false);
}

SCENARIO("accumulate")
{
  std::vector<int> v{1,2,3,4,5};
  REQUIRE(ezy::accumulate(v, 0) == 15);
  REQUIRE(ezy::accumulate(v, 5) == 20);

  REQUIRE(ezy::accumulate(v, 0, std::minus{}) == -15);
  REQUIRE(ezy::accumulate(v, 1, std::multiplies{}) == 120);
}

SCENARIO("accumulate works with member function")
{
  struct string
  {
    std::string m;

    string& append(const std::string& str)
    {
      m += str;
      return *this;
    }
  };
  std::vector<std::string> v{"Hello", " ", "world!"};
  REQUIRE(ezy::accumulate(v, string{"#"}, &string::append).m == "#Hello world!");
}

SCENARIO("accumulate in compile time")
{
  constexpr std::array a{1,2,3,4,5};
  static_assert(ezy::accumulate(a, 0) == 15);
  static_assert(ezy::accumulate(a, 1, std::multiplies{}) == 120);
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

SCENARIO("collecting to iterator")
{
  std::vector<int> v{1,2,4,8,16};
  std::vector<int> dest(v.size(), {});
  ezy::collect(v, std::begin(dest));
  REQUIRE(dest == v);
}

SCENARIO("collecting to iterator returns iterator")
{
  std::vector<int> v1{1,3,5,7,9};
  std::vector<int> v2{2,4,6,8,10};
  std::vector<int> dest(v1.size() + v2.size(), {});
  const auto it = ezy::collect(v1, std::begin(dest));
  ezy::collect(v2, it);
  REQUIRE(dest == std::vector{1,3,5,7,9,2,4,6,8,10});
}

SCENARIO("iterate")
{
  const auto it = ezy::iterate(1, [](int i) { return i * 2;});
  const auto joined = join_as_strings(ezy::take(it, 7), ",");
  REQUIRE(joined == "1,2,4,8,16,32,64");
}

SCENARIO("iterate increments by one as a default")
{
  const auto it = ezy::iterate(4);
  const auto joined = join_as_strings(ezy::take(it, 5), ",");
  REQUIRE(joined == "4,5,6,7,8");
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


SCENARIO("enumerator has proper type")
{
  std::vector<double> doubles{1.2, 3.4};
  const auto enumerated = ezy::enumerate(doubles);
  static_assert(std::is_same_v<std::remove_reference_t<decltype(std::get<0>(*enumerated.begin()))>, size_t>);
}

SCENARIO("enumerate on classic array")
{
  double doubles[] = {1.2, 3.4};
  const auto enumerated = ezy::enumerate(doubles);
  static_assert(std::is_same_v<std::remove_reference_t<decltype(std::get<0>(*enumerated.begin()))>, size_t>);
}

SCENARIO("cycle")
{
  const auto cycled = ezy::cycle(std::vector{2, 3, 4});
  const auto joined = join_as_strings(ezy::take(cycled, 7), ",");
  REQUIRE(joined == "2,3,4,2,3,4,2");
}

SCENARIO("cycle works with array")
{
  const int array[] = {3, 4, 5};
  const auto cycled = ezy::cycle(array);
  const auto joined = join_as_strings(ezy::take(cycled, 7), ",");
  REQUIRE(joined == "3,4,5,3,4,5,3");
}

SCENARIO("repeat")
{
  const auto repeated = ezy::repeat(4);
  const auto joined = join_as_strings(ezy::take(repeated, 5), ",");
  REQUIRE(joined == "4,4,4,4,4");
}

SCENARIO("chunk")
{
  const std::vector<int> v{1,2,3,4,5,6,7,8,9};
  const auto chunks = ezy::chunk(v, 4);
  REQUIRE(ezy::size(chunks) == 3);
  auto it = std::begin(chunks);
  REQUIRE(join_as_strings(*it, ",") == "1,2,3,4");
  REQUIRE(join_as_strings(*std::next(it, 1), ",") == "5,6,7,8");
  REQUIRE(join_as_strings(*std::next(it, 2), ",") == "9");
  REQUIRE(std::next(it, 3) == std::end(chunks));

  REQUIRE(join_as_strings(ezy::flatten(chunks)) == "123456789");
}

SCENARIO("chunk works with array")
{
  const int a[] = {1,2,3,4,5,6,7,8,9};
  const auto chunks = ezy::chunk(a, 4);
  const auto size = ezy::size(chunks);
  REQUIRE(size == 3);
  // TODO static_assert(std::is_same<decltype(size), size_t>::value); // bug size_type_t and ezy::size returns
  // different type!
  auto it = std::begin(chunks);
  REQUIRE(join_as_strings(*it, ",") == "1,2,3,4");
  REQUIRE(join_as_strings(*std::next(it, 1), ",") == "5,6,7,8");
  REQUIRE(join_as_strings(*std::next(it, 2), ",") == "9");
  REQUIRE(std::next(it, 3) == std::end(chunks));

  REQUIRE(join_as_strings(ezy::flatten(chunks)) == "123456789");
}

SCENARIO("range(until)")
{
  GIVEN("a range until 0")
  {
    const auto r = ezy::range(0);
    REQUIRE(ezy::empty(r));
  }

  GIVEN("a range until 1")
  {
    const auto r = ezy::range(1);
    REQUIRE(join_as_strings(r, ",") == "0");
  }

  GIVEN("a range until 10")
  {
    const auto r = ezy::range(10);
    REQUIRE(join_as_strings(r, ",") == "0,1,2,3,4,5,6,7,8,9");
  }

  GIVEN("a range until 4.8")
  {
    const auto r = ezy::range(4.8);
    REQUIRE(join_as_strings(r, ",") == "0.000000,1.000000,2.000000,3.000000,4.000000");
    static_assert(std::is_same<ezy::detail::value_type_t<decltype(r)>, double>::value);
  }

  GIVEN("a range specified by lvalue")
  {
    int i = 6;
    const auto r = ezy::range(i);
    REQUIRE(join_as_strings(r, ",") == "0,1,2,3,4,5");
    static_assert(std::is_same<ezy::detail::value_type_t<decltype(r)>, int>::value);
  }
}

SCENARIO("range(from, until)")
{
  GIVEN("[0, 0)")
  {
    const auto r = ezy::range(0, 0);
    REQUIRE(ezy::empty(r));
  }

  GIVEN("[0, 1)")
  {
    const auto r = ezy::range(0, 1);
    REQUIRE(join_as_strings(r, ",") == "0");
  }

  GIVEN("[4, 8)")
  {
    const auto r = ezy::range(4, 8);
    REQUIRE(join_as_strings(r, ",") == "4,5,6,7");
  }

  GIVEN("['a', 'f')")
  {
    const auto r = ezy::range('a', 'f');
    REQUIRE(ezy::join<std::string>(r, ",") == "a,b,c,d,e");
  }

  GIVEN("[1.2, 4.3]")
  {
    const auto r = ezy::range(1.2, 4.3);
    REQUIRE(join_as_strings(r, ",") == "1.200000,2.200000,3.200000,4.200000");
    static_assert(std::is_same<ezy::detail::value_type_t<decltype(r)>, double>::value);
  }
}

SCENARIO("range(from, until, step)")
{
  GIVEN("[2 +3, 15]")
  {
    const auto r = ezy::range(2, 15, 3);
    REQUIRE(join_as_strings(r, ",") == "2,5,8,11,14");
  }

  GIVEN("[2 -3, -15]")
  {
    const auto r = ezy::range(2, -17, -3);
    REQUIRE(join_as_strings(r, ",") == "2,-1,-4,-7,-10,-13,-16");
  }
}

SCENARIO("split")
{
  GIVEN("a sentence")
  {
    std::string in("This is a sentence.");
    const auto r = ezy::split(in, ' ');
    REQUIRE(ezy::join(r, "+") == "This+is+a+sentence.");
  }

  GIVEN("a string where the last character is a delimiter")
  {
    std::string in("This is a sentence. ");
    const auto r = ezy::split(in, ' ');
    REQUIRE(ezy::join(r, "+") == "This+is+a+sentence.");
  }

  GIVEN("a string where the first character is a delimiter")
  {
    std::string in(" This is a sentence. ");
    const auto r = ezy::split(in, ' ');
    REQUIRE(ezy::join(r, "+") == "This+is+a+sentence.");
  }

  GIVEN("a string where there are repeating delimiters")
  {
    std::string in("   This  is    a   sentence.  ");
    const auto r = ezy::split(in, ' ');
    REQUIRE(ezy::join(r, "+") == "This+is+a+sentence.");
  }
}
