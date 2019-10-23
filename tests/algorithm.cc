#include <catch.hpp>

#include <ezy/algorithm>

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

  GIVEN("a type with begin and end")
  {
    REQUIRE(ezy::empty(iterable_t{}));
    REQUIRE(!ezy::empty(iterable_t{1}));
  }

  GIVEN("a type with external begin and end")
  {
    REQUIRE(ezy::empty(external_iterable_t{}));
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
