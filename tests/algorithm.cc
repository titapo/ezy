#include <catch.hpp>

#include <ezy/algorithm>
#include <ezy/strong_type>
#include <ezy/features/iterable.h>

#include <vector>

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

template <typename Range>
std::string
range_to_string(Range&& range)
{
  std::string r;
  ezy::for_each(std::forward<Range>(range), [&r](int i) { r += std::to_string(i); });
  return r;
}


// TODO ezy::join()
template <typename Range>
std::string
join(Range&& range)
{
  std::string r;
  ezy::for_each(std::forward<Range>(range), [&r](auto i) { r += i; });
  return r;
}

template <typename Range>
std::string
join_as_strings(Range&& range)
{
  return join(ezy::transform(std::forward<Range>(range), [](auto e) { return std::to_string(e); }));
}

SCENARIO("transform")
{
  std::vector<int> v{1,2,3};
  std::string r;
  auto mapped = ezy::transform(v, [](int i) { return i + 1; });
  ezy::for_each(mapped, [&r](int i) { r += std::to_string(i); });
  REQUIRE(r == "234");
}

SCENARIO("concatenate")
{
  std::vector<int> v1{1,2,3};
  std::vector<int> v2{4,5,6};
  const auto concatenated = ezy::concatenate(v1, v2);
  REQUIRE(range_to_string(concatenated) == "123456");
}

SCENARIO("concatenate - when the first is a temporary")
{
  std::vector<int> v2{4,5,6};
  const auto concatenated = ezy::concatenate(std::vector{1,2,3}, v2);
  REQUIRE(range_to_string(concatenated) == "123456");
}

SCENARIO("concatenate - when the second is a temporary")
{
  std::vector<int> v1{1,2,3};
  const auto concatenated = ezy::concatenate(v1, std::vector{4,5,6});
  REQUIRE(range_to_string(concatenated) == "123456");
}

SCENARIO("concatenate - when the both are temporaries")
{
  const auto concatenated = ezy::concatenate(std::vector{1,2,3}, std::vector{4,5,6});
  REQUIRE(range_to_string(concatenated) == "123456");
}

SCENARIO("zip")
{
  std::vector<int> v1{1,2,3};
  std::vector<int> v2{4,5,6};
  const auto zipped = ezy::zip(v1, v2);
  const auto joined = join(
      ezy::transform(
        zipped,
        [](auto pair) -> std::string {auto[a,b] = pair; return std::to_string(a) + "+" + std::to_string(b) + ";"; }
  ));
  REQUIRE(joined == "1+4;2+5;3+6;");
}

SCENARIO("zip with temporaries")
{
  const auto zipped = ezy::zip(std::vector{1,2,3}, std::vector{4,5,6});
  const auto joined = join(
      ezy::transform(
        zipped,
        [](auto pair) -> std::string {auto[a,b] = pair; return std::to_string(a) + "+" + std::to_string(b) + ";"; }
  ));
  REQUIRE(joined == "1+4;2+5;3+6;");
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
