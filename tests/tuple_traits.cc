#include <catch2/catch.hpp>

#include <ezy/tuple_traits.h>

#include <variant>

template <typename, typename>
struct integral_constant_add;

template <typename T, T N1, T N2>
struct integral_constant_add<std::integral_constant<T, N1>, std::integral_constant<T, N2>>
{
  using type = std::integral_constant<T, N1 + N2>;
};

template <typename T1, typename T2>
using integral_constant_add_t = typename integral_constant_add<T1, T2>::type;

template <size_t N>
using index_constant = std::integral_constant<size_t, N>;

template <typename>
struct unary_template {};

template <typename...>
struct types;

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

    static_assert(std::is_same_v<ett::append_t<types<>, int>, types<int>>);
    static_assert(std::is_same_v<ett::append_t<types<int>, double>, types<int, double>>);
  }

  GIVEN("extend")
  {
    static_assert(std::is_same_v<ett::extend_t<types<>>, types<>>);
    static_assert(std::is_same_v<ett::extend_t<types<int, int>>, types<int, int>>);
    static_assert(std::is_same_v<ett::extend_t<types<int, int>, types<double>>, types<int, int, double>>);
    static_assert(std::is_same_v<ett::extend_t<types<int, int>, types<double>, types<bool>>,
        types<int, int, double, bool>>
        );
  }

  GIVEN("head")
  {
    //static_assert(std::is_same_v<ett::types<std::tuple<>>, void>); // OK: it does not compile
    static_assert(std::is_same_v<ett::head_t<types<int>>, int>);
    static_assert(std::is_same_v<ett::head_t<types<double, int>>, double>);
    static_assert(std::is_same_v<ett::head_t<types<double, bool, int>>, double>);
  }

  GIVEN("tail")
  {
    //static_assert(std::is_same_v<ett::tail_t<types<>>, void>); // OK: it does not compile
    static_assert(std::is_same_v<ett::tail_t<types<int>>, types<>>);
    static_assert(std::is_same_v<ett::tail_t<types<double, int>>, types<int>>);
    static_assert(std::is_same_v<ett::tail_t<types<double, bool, int>>, types<bool, int>>);
  }

  GIVEN("remove")
  {
    static_assert(std::is_same_v<ett::remove_t<types<>, int>, types<>>);
    static_assert(std::is_same_v<ett::remove_t<types<int>, int>, types<>>);
    static_assert(std::is_same_v<ett::remove_t<types<int>, double>, types<int>>);
    static_assert(std::is_same_v<ett::remove_t<types<int, double, int, double>, double>, types<int, int>>);
  }

  GIVEN("any_of")
  {
    static_assert(ett::any_of_v<types<>, std::is_reference> == false);
    static_assert(ett::any_of_v<types<int>, std::is_reference> == false);
    static_assert(ett::any_of_v<types<int, bool&>, std::is_reference> == true);
    static_assert(ett::any_of_v<types<int&>, std::is_reference> == true);
  }

  GIVEN("none_of")
  {
    static_assert(ett::none_of_v<types<>, std::is_reference> == true);
    static_assert(ett::none_of_v<types<int>, std::is_reference> == true);
    static_assert(ett::none_of_v<types<int, bool&>, std::is_reference> == false);
    static_assert(ett::none_of_v<types<int&>, std::is_reference> == false);
  }

  GIVEN("all_of")
  {
    static_assert(ett::all_of_v<types<>, std::is_reference> == true);
    static_assert(ett::all_of_v<types<int>, std::is_reference> == false);
    static_assert(ett::all_of_v<types<int, bool&>, std::is_reference> == false);
    static_assert(ett::all_of_v<types<int&>, std::is_reference> == true);
  }

  GIVEN("contains")
  {
    static_assert(ett::contains_v<types<>, int> == false);
    static_assert(ett::contains_v<types<int>, int> == true);
    static_assert(ett::contains_v<types<double, int>, int> == true);
    static_assert(ett::contains_v<types<double, int>, char> == false);
  }

  GIVEN("filter")
  {
    static_assert(std::is_same_v<ett::filter_t<types<>, std::is_reference>, types<>>);
    static_assert(std::is_same_v<ett::filter_t<types<char&, int, int&>, std::is_reference>, types<char&, int&>>);
  }

  GIVEN("unique")
  {
    static_assert(std::is_same_v<ett::unique_t<types<>>, types<>>);
    static_assert(std::is_same_v<ett::unique_t<types<int, double>>, types<int, double>>);
    static_assert(std::is_same_v<ett::unique_t<types<int, int>>, types<int>>);
    static_assert(std::is_same_v<ett::unique_t<types<int, double, double, int, char, int>>, types<int, double, char>>);
  }

  GIVEN("flatten")
  {
    static_assert(std::is_same_v<ett::flatten_t<types<types<>>>, types<>>);
    static_assert(std::is_same_v<ett::flatten_t<types<types<int>, types<bool>>>, types<int, bool>>);
    static_assert(std::is_same_v<ett::flatten_t<types<types<int, double>, types<bool, int>>>, types<int, double, bool, int>>);
  }

  GIVEN("subtract")
  {
    static_assert(std::is_same_v<ett::subtract_t<types<>, types<>>, types<>>);
    static_assert(std::is_same_v<ett::subtract_t<types<int>, types<>>, types<int>>);
    static_assert(std::is_same_v<ett::subtract_t<types<int>, types<int>>, types<>>);
    static_assert(std::is_same_v<ett::subtract_t<types<>, types<int>>, types<>>);
  }

  GIVEN("map")
  {
    static_assert(std::is_same_v<ett::map_t<types<>, unary_template>, types<>>);
    static_assert(std::is_same_v<ett::map_t<types<int>, unary_template>, types<unary_template<int>>>);
    static_assert(std::is_same_v<ett::map_t<types<int, char, int>, unary_template>,
        types<unary_template<int>, unary_template<char>, unary_template<int>>
        >);
  }

  GIVEN("fold")
  {
    static_assert(std::is_same_v<
        ett::fold_t<types<>, index_constant<0>, integral_constant_add_t>,
        index_constant<0>>);

    static_assert(std::is_same_v<
        ett::fold_t<types<index_constant<3>>, index_constant<0>, integral_constant_add_t>,
        index_constant<3>>);

    static_assert(std::is_same_v<
        ett::fold_t<types<index_constant<3>, index_constant<5>>, index_constant<0>, integral_constant_add_t>,
        index_constant<8>>);
  }

  GIVEN("zip")
  {
    static_assert(std::is_same_v<ett::zip_t<types<>, types<>>, types<>>);
    static_assert(std::is_same_v<ett::zip_t<types<int>, types<bool>>,
        types<types<int, bool>>>);

    static_assert(std::is_same_v<ett::zip_t<types<int, double>, types<char, bool>>,
        types<types<int, char>, types<double, bool>>>);
  }

  GIVEN("enumerate")
  {
    static_assert(std::is_same_v<ett::enumerate_t<std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<ett::enumerate_t<std::tuple<int>>,
        std::tuple< std::tuple<std::integral_constant<size_t, 0>, int> >>);

    static_assert(std::is_same_v<ett::enumerate_t<std::tuple<int, double, int>>,
        std::tuple<
          std::tuple<std::integral_constant<size_t, 0>, int>,
          std::tuple<std::integral_constant<size_t, 1>, double>,
          std::tuple<std::integral_constant<size_t, 2>, int>
        >>);
  }

  GIVEN("rebind") // or rewrap?
  {
    static_assert(std::is_same_v<ett::rebind_t<std::tuple<int>, std::variant>, std::variant<int> >);
  }

  GIVEN("extract")
  {
    static_assert(std::is_same_v<ett::extract_t<std::tuple<int, double, bool>>, ezy::typelist<int, double, bool>>);
  }
}
