#include <catch.hpp>

#include <ezy/tuple_traits>

#include <variant>

SCENARIO("tuple_traits")
{
  namespace ett = ezy::tuple_traits;

  GIVEN("tuplify")
  {
    static_assert(std::is_same_v<ett::tuplify_t<std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<ett::tuplify_t<std::tuple<int>>, std::tuple<int>>);
    static_assert(std::is_same_v<ett::tuplify_t<std::tuple<int, double>>, std::tuple<int, double>>);
    static_assert(std::is_same_v<ett::tuplify_t<int>, std::tuple<int>>);
    static_assert(std::is_same_v<ett::tuplify_t<void>, std::tuple<>>);
  }

  GIVEN("append")
  {
    static_assert(std::is_same_v<ett::append_t<std::tuple<>, int>, std::tuple<int>>);
    static_assert(std::is_same_v<ett::append_t<std::tuple<int>, double>, std::tuple<int, double>>);
  }

  GIVEN("extend")
  {
    static_assert(std::is_same_v<ett::extend_t<>, std::tuple<>>);
    static_assert(std::is_same_v<ett::extend_t<std::tuple<int, int>>, std::tuple<int, int>>);
    static_assert(std::is_same_v<ett::extend_t<std::tuple<int, int>, std::tuple<double>>, std::tuple<int, int, double>>);
    static_assert(std::is_same_v<ett::extend_t<std::tuple<int, int>, std::tuple<double>, std::tuple<bool>>,
        std::tuple<int, int, double, bool>>
        );
  }

  GIVEN("head")
  {
    //static_assert(std::is_same_v<ett::head_t<std::tuple<>>, void>); // OK: it does not compile
    static_assert(std::is_same_v<ett::head_t<std::tuple<int>>, int>);
    static_assert(std::is_same_v<ett::head_t<std::tuple<double, int>>, double>);
    static_assert(std::is_same_v<ett::head_t<std::tuple<double, bool, int>>, double>);
  }

  GIVEN("tail")
  {
    //static_assert(std::is_same_v<ett::tail_t<std::tuple<>>, void>); // OK: it does not compile
    static_assert(std::is_same_v<ett::tail_t<std::tuple<int>>, std::tuple<>>);
    static_assert(std::is_same_v<ett::tail_t<std::tuple<double, int>>, std::tuple<int>>);
    static_assert(std::is_same_v<ett::tail_t<std::tuple<double, bool, int>>, std::tuple<bool, int>>);
  }

  GIVEN("remove")
  {
    static_assert(std::is_same_v<ett::remove_t<std::tuple<>, int>, std::tuple<>>);
    static_assert(std::is_same_v<ett::remove_t<std::tuple<int>, int>, std::tuple<>>);
    static_assert(std::is_same_v<ett::remove_t<std::tuple<int>, double>, std::tuple<int>>);
    static_assert(std::is_same_v<ett::remove_t<std::tuple<int, double, int, double>, double>, std::tuple<int, int>>);
  }

  GIVEN("any_of")
  {
    static_assert(ett::any_of_v<std::tuple<>, std::is_reference> == false);
    static_assert(ett::any_of_v<std::tuple<int>, std::is_reference> == false);
    static_assert(ett::any_of_v<std::tuple<int, bool&>, std::is_reference> == true);

  }

  GIVEN("contains")
  {
    static_assert(ett::contains_v<std::tuple<>, int> == false);
    static_assert(ett::contains_v<std::tuple<int>, int> == true);
    static_assert(ett::contains_v<std::tuple<double, int>, int> == true);
    static_assert(ett::contains_v<std::tuple<double, int>, char> == false);
  }

  GIVEN("filter")
  {
    static_assert(std::is_same_v<ett::filter_t<std::tuple<>, std::is_reference>, std::tuple<>>);
    static_assert(std::is_same_v<ett::filter_t<std::tuple<char&, int, int&>, std::is_reference>, std::tuple<char&, int&>>);
  }

  GIVEN("unique")
  {
    static_assert(std::is_same_v<ett::unique_t<std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<ett::unique_t<std::tuple<int, double>>, std::tuple<int, double>>);
    static_assert(std::is_same_v<ett::unique_t<std::tuple<int, int>>, std::tuple<int>>);
    static_assert(std::is_same_v<ett::unique_t<std::tuple<int, double, double, int, char, int>>, std::tuple<int, double, char>>);
  }

  GIVEN("flatten")
  {
    static_assert(std::is_same_v<ett::flatten_t<std::tuple<std::tuple<>>>, std::tuple<>>);
    static_assert(std::is_same_v<ett::flatten_t<std::tuple<std::tuple<int>, std::tuple<bool>>>, std::tuple<int, bool>>);
    static_assert(std::is_same_v<ett::flatten_t<std::tuple<std::tuple<int, double>, std::tuple<bool, int>>>, std::tuple<int, double, bool, int>>);
  }

  GIVEN("subtract")
  {
    static_assert(std::is_same_v<ett::subtract_t<std::tuple<>, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<ett::subtract_t<std::tuple<int>, std::tuple<>>, std::tuple<int>>);
    static_assert(std::is_same_v<ett::subtract_t<std::tuple<int>, std::tuple<int>>, std::tuple<>>);
    static_assert(std::is_same_v<ett::subtract_t<std::tuple<>, std::tuple<int>>, std::tuple<>>);
  }

  GIVEN("map")
  {
    static_assert(std::is_same_v<ett::map_t<std::tuple<>, std::vector>, std::tuple<>>);
    static_assert(std::is_same_v<ett::map_t<std::tuple<int>, std::vector>, std::tuple<std::vector<int>>>);
    static_assert(std::is_same_v<ett::map_t<std::tuple<int, char, int>, std::vector>,
        std::tuple<std::vector<int>, std::vector<char>, std::vector<int>>
        >);
  }

  GIVEN("rebind") // or rewrap?
  {
    static_assert(std::is_same_v<ett::rebind_t<std::tuple<int>, std::variant>, std::variant<int> >);
  }

  GIVEN("extract")
  {
    static_assert(std::is_same_v<ett::extract_t<std::tuple<int, double, bool>>, std::tuple<int, double, bool>>);
  }
}
