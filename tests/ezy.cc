#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include "../include/ezy/strong_type.h"
#include "../include/ezy/Enumeration.h"

/**
 * strong type features: iterable
 */

template <typename RangeType>
auto range_size(const RangeType& range)
{
  return std::distance(std::begin(range), std::end(range));
}

template <typename ElementType>
std::ostream& element_to_stream(std::ostream& ostr, const ElementType& element)
{
  return ostr << element;
}

template <typename RangeType>
std::ostream& range_to_stream(std::ostream& ostr, const RangeType& range)
{
  ostr << "[";
  for (const auto e : range)
    element_to_stream(ostr, e) << ", ";

  return ostr << "]";
}

template <typename RangeType>
std::string range_to_string(const RangeType& range)
{
  std::ostringstream ostr;
  range_to_stream(ostr, range);
  return ostr.str();
}

#define COMPARE_RANGES(a, b) REQUIRE(range_to_string(a) == range_to_string(b));

// TODO (tests) detect if it is a range
enum class ResultIs
{
  Range,
  Value
};

enum class Evaluated
{
  Eager,
  Lazy
};

template <typename ActionType, ResultIs Result = ResultIs::Range, Evaluated Eval = Evaluated::Lazy>
void check_evaluation(ActionType&& action)
{
  using MyNumbers = strong_type<std::vector<int>, struct MyNumbersTag, iterable>;
  MyNumbers numbers{1,2,3,4,5,6,7,8,9,10};
  const auto result = action(numbers);
  const auto new_numbers = MyNumbers{42, 43, 44, 45};
  numbers = new_numbers; //iterators invalidated
  if constexpr (Result == ResultIs::Range)
  {
    COMPARE_RANGES(result, action(new_numbers));
  }
  else
  {
    if constexpr (Eval == Evaluated::Lazy)
    {
      REQUIRE(result == action(new_numbers));
    }
    else
    {
      REQUIRE(result != action(new_numbers));
    }
  }
}

template <typename ActionType>
void check_lazy_evaluation(ActionType&& action)
{
  check_evaluation(std::forward<ActionType>(action));
}

template <typename ActionType>
void check_lazy_evaluation_with_value(ActionType&& action)
{
  check_evaluation<ActionType, ResultIs::Value>(std::forward<ActionType>(action));
}

template <typename ActionType>
void check_eager_evaluation_with_value(ActionType&& action)
{
  check_evaluation<ActionType, ResultIs::Value, Evaluated::Eager>(std::forward<ActionType>(action));
}


SCENARIO("strong type extensions")
{
  using MyNumbers = strong_type<std::vector<int>, struct MyNumbersTag, iterable>;
  GIVEN("some numbers")
  {
    MyNumbers numbers{1,2,3,4,5,6,7,8,9,10};
    WHEN("nothing happens")
    {
      THEN("remain the same")
      {
        COMPARE_RANGES(numbers, (std::initializer_list{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
      }
    }

    WHEN("elements printed with foreach")
    {
      std::ostringstream ostr;
      const auto print = [&ostr](const auto& e) { ostr << e; };
      numbers.for_each(print);
      THEN ("contains all element")
      {
        REQUIRE(ostr.str() == "12345678910");
      }
    }

    WHEN("elements mapped")
    {
      const auto plusTwo = [](const int& e) { return e + 2; };
      const auto &result = numbers.map(plusTwo);
      THEN ("result contains all element incremented by 2")
      {
        COMPARE_RANGES(result, (std::initializer_list{3, 4, 5, 6, 7, 8, 9, 10, 11, 12}));
      }
    }

    WHEN("elements filtered")
    {
      const auto isEven = [](const auto& e) { return e % 2 == 0; };
      const auto &result = numbers.filter(isEven);
      THEN ("result contains even numbers")
      {
        COMPARE_RANGES(result, (std::initializer_list{2, 4, 6, 8, 10}));
      }
    }

    WHEN("elements partitioned")
    {
      const auto isEven = [](const auto& e) { return e % 2 == 0; };
      const auto &[resultTrue, resultFalse] = numbers.partition(isEven);
      THEN ("result contains even numbers")
      {
        COMPARE_RANGES(resultTrue, (std::initializer_list{2, 4, 6, 8, 10}));
        COMPARE_RANGES(resultFalse, (std::initializer_list{1, 3, 5, 7, 9}));
      }
    }

    WHEN("elements sliced within boundaries")
    {
      const auto result = numbers.slice(1, 3);
      THEN("contains the sliced elements")
      {
        COMPARE_RANGES(result, (std::initializer_list{2, 3}));
      }
    }

    WHEN("elements sliced with larger ending boundary")
    {
      const auto result = numbers.slice(4, 15);
      THEN("contains the sliced elements")
      {
        COMPARE_RANGES(result, (std::initializer_list{5, 6, 7, 8, 9, 10}));
      }
    }

    WHEN("elements sliced with after starting boundary")
    {
      const auto result = numbers.slice(12, 14);
      THEN("contains the sliced elements")
      {
        COMPARE_RANGES(result, (std::initializer_list<int>{}));
      }
    }

    WHEN("elements sliced with reverse interval")
    {
      REQUIRE_THROWS_AS(numbers.slice(4, 2), std::logic_error);
    }

    WHEN("elements sliced with after boundaries")
    {
      const auto result = numbers.slice(11, 15);
      THEN("contains the sliced elements")
      {
        COMPARE_RANGES(result, (std::initializer_list<int>{}));
      }
    }

    WHEN("another range concatenated")
    {
      const std::vector<int> others{3, 4, 5};
      const auto result = numbers.concatenate(others);

      THEN("contains all elements")
      {
        COMPARE_RANGES(result, (std::initializer_list{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 3, 4, 5}));
      }
    }


    const auto less_than = [](const auto& rhs)
    {
      return [rhs](const auto& lhs)
      { return lhs < rhs; };
    };

    WHEN("all() called")
    {
      THEN("it is OK")
      {
        REQUIRE(numbers.all(less_than(11)));
        REQUIRE_FALSE(numbers.all(less_than(10)));
        REQUIRE_FALSE(numbers.all(less_than(-1)));
      }
    }

    WHEN("any() called")
    {
      THEN("it is OK")
      {
        REQUIRE(numbers.any(less_than(11)));
        REQUIRE(numbers.any(less_than(10)));
        REQUIRE_FALSE(numbers.any(less_than(1)));
        REQUIRE_FALSE(numbers.any(less_than(-1)));
      }
    }

    WHEN("none() called")
    {
      THEN("it is OK")
      {
        REQUIRE_FALSE(numbers.none(less_than(11)));
        REQUIRE_FALSE(numbers.none(less_than(10)));
        REQUIRE(numbers.none(less_than(1)));
        REQUIRE(numbers.none(less_than(-1)));
      }
    }

    WHEN("converted to vector")
    {
      const auto result = numbers.to<std::vector<int>>();
      THEN("it is ok")
      {
        COMPARE_RANGES(result, (std::initializer_list{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
      }
    }

    const auto stringify_pair = [](const auto& p)
      { return std::to_string(p.first) + ":" + std::to_string(p.second); };

    WHEN("converted to map")
    {
      const auto result = numbers
        .map([](const int i) { return std::make_pair(i, 2 * i); })
        .to_iterable<std::map<int, int>>();

      THEN("it is ok")
      {
        const auto as_strings = result.map(stringify_pair);
        COMPARE_RANGES(as_strings, (std::initializer_list<std::string>{"1:2", "2:4", "3:6", "4:8", "5:10", "6:12", "7:14", "8:16", "9:18", "10:20"}));
      }
    }

    const auto to_string = [](auto&& e) { return std::to_string(e); }; // lifting
    WHEN("zipped")
    {
      const auto add_percent = [](const std::string& s) { return s + "%"; };
      const auto numbers_as_strings = numbers
        .map([](auto i) { return 2*i; })
        .map(to_string)
        .map(add_percent)
        .to<std::vector<std::string>>();

      const auto result = numbers.zip(numbers_as_strings);

      THEN("it is ok")
      {
        const auto as_strings = result
          .map([](const auto& p) { return std::to_string(p.first) + ":" + p.second; });

        COMPARE_RANGES(as_strings,
            (std::initializer_list<std::string>{"1:2%", "2:4%", "3:6%", "4:8%", "5:10%", "6:12%", "7:14%", "8:16%", "9:18%", "10:20%"}));
      }
    }

    WHEN("find existing")
    {
      const auto result = numbers.find(less_than(5));
      REQUIRE(result.has_value());
      REQUIRE(result.value_or(99) == 1);
    }

    WHEN("find non existing")
    {
      const auto result = numbers.find(less_than(0));
      REQUIRE(!result.has_value());
      REQUIRE(result.value_or(99) == 99);
    }

    WHEN("contains element")
    {
      REQUIRE(numbers.contains(3));
      REQUIRE(!numbers.contains(-3));
    }

    WHEN("accumulated")
    {
      REQUIRE(MyNumbers{}.accumulate(10) == 10);
      REQUIRE(numbers.accumulate(0) == 55);
      REQUIRE(numbers.accumulate(0, std::minus<int>{}) == -55);
    }

    /*
    WHEN("reduced")
    {
      REQUIRE(MyNumbers{}.reduce(0));
      // TODO empty reduce
      //REQUIRE(numbers.reduce(0) == 56);
    }
    */
   
    // TODO flatten
    WHEN("flattened")
    {
      // FIXME it seems that MyNumbers has no const_iterator
      using Raised = strong_type< std::vector<std::vector<int>>, struct RaisedTag, iterable>;

      COMPARE_RANGES((Raised{std::vector{1, 2, 3}, std::vector{4, 5, 6}}).flatten(),
          (std::initializer_list{1, 2, 3, 4, 5, 6}));

      COMPARE_RANGES((Raised{std::vector{1, 2, 3}, std::vector<int>{}, std::vector<int>{}, std::vector{4, 5, 6}}).flatten(),
          (std::initializer_list{1, 2, 3, 4, 5, 6}));

      COMPARE_RANGES(Raised{}.flatten(),
          (std::initializer_list<int>{}));
    }

    /*
    WHEN("elements flatmapped")
    {
      const auto around = [](const auto& e) { return std::initializer_list{e - 1, e, e + 1}; };
      const auto result = numbers.flat_map(around);
      THEN("contains flat list")
      {
        COMPARE_RANGES(result, (std::initializer_list{0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4, 5, 4, 5, 6}));
      }
    }
    */

    //
    // TODO fold
    // TODO span(unary): similar to partition, just splitting at first !unary(element)
    // TODO break(unary): span with inverted condition
    // TODO drop(n: index): drop (and return) the first n elements <-- what is the difference between this and take
    //                     (mutability?)
    // TODO split_at(index)
    // TODO head/tail
    // TODO take(n)?, take_while(predicate)
    // TODO unzip
    // TODO grouping?
    // TODO collect?
    // TODO empty
    //

    // TODO this does not work. Some weird iteration invalidation is suspected
    /*
    WHEN("mapped then filtered")
    {
      auto transform = [](auto i) { return i + 10; };
      auto is_even = [](auto i) { return (i % 2) == 0; };

      const auto result = numbers
        .map(transform)
        .filter(is_even)
        ;
      std::cout << "xxx"<< std::endl;

      for (const auto& e : result)
        std::cout << e << std::endl;

      std::cout << "xxx2"<< std::endl;
      for (const auto& e : result) // This segfaults
        std::cout << e << std::endl;
      std::cout << "this is done " << &result << "\n"<< std::endl;
      //const auto expected = (std::initializer_list<int>{12,14,16,18,20});
      //REQUIRE(std::equal(result.begin(), result.end(), expected.begin(), expected.end()));
      COMPARE_RANGES(result, (std::initializer_list<int>{12,14,16,18,20 ,99})); // this also segfaults
    }
    */
  }

}

// lazily evaluated
TEST_CASE(".map() is lazily evaluated")
{
  const auto plusOne = [](const auto& i) {return i + 1;};
  check_lazy_evaluation([&plusOne](const auto& range) { return range.map(plusOne); });
}

TEST_CASE(".filter() is lazily evaluated")
{
  const auto isEven = [](const auto& i) {return (i % 2) == 0;};
  check_lazy_evaluation([&isEven](const auto& range) { return range.filter(isEven); });
}

TEST_CASE(".slice() is lazily evaluated")
{
  check_lazy_evaluation([](const auto& range) { return range.slice(2, 15); });
}

// eager evaluated

TEST_CASE(".any() is eager evaluated")
{
  const auto biggerThan20 = [](const auto& i) {return i > 20;};
  check_eager_evaluation_with_value([&](const auto& range) { return range.any(biggerThan20); });
}

TEST_CASE(".all() is eager evaluated")
{
  const auto biggerThan20 = [](const auto& i) {return i > 20;};
  check_eager_evaluation_with_value([&](const auto& range) { return range.all(biggerThan20); });
}

TEST_CASE(".none() is eager evaluated")
{
  const auto biggerThan20 = [](const auto& i) {return i > 20;};
  check_eager_evaluation_with_value([&](const auto& range) { return range.none(biggerThan20); });
}

// optional

SCENARIO("optional")
{
  GIVEN("some optionals")
  {
    Optional<int> opt_0 = 0;
    Optional<int> opt_1 = 1;
    Optional<int> opt_n = std::nullopt;

    WHEN("asked has_value")
    {
      THEN("proper values returned")
      {
        REQUIRE(opt_0.has_value() == true);
        REQUIRE(opt_1.has_value() == true);
        REQUIRE(opt_n.has_value() == false);
      }
    }

    WHEN("asked value_or")
    {
      THEN("proper values returned")
      {
        REQUIRE(opt_0.value_or(123) == 0);
        REQUIRE(opt_1.value_or(123) == 1);
        REQUIRE(opt_n.value_or(123) == 123);
      }
    }

    WHEN("they are matched")
    {
      const auto matcher = ezy::overloaded{
        [](int i) -> std::string { return std::to_string(1000 + i);},
        [](std::nullopt_t) -> std::string { return "none";},
      };
      THEN("proper values returned")
      {
        REQUIRE(opt_0.match(matcher) == "1000");
        REQUIRE(opt_1.match(matcher) == "1001");
        REQUIRE(opt_n.match(matcher) == "none");
      }
    }

    WHEN("they are mapped")
    {
      const auto stringify = [](int i) { return std::to_string(i);}; // just lifting

      THEN("positive values are mapped actually")
      {
        REQUIRE(opt_0.map(stringify).value_or("FAILED") == "0");
        REQUIRE(opt_1.map(stringify).value_or("FAILED") == "1");
        REQUIRE(opt_n.map(stringify).has_value() == false);
      }
    }

    WHEN("they are map_or-ed")
    {
      const auto stringify = [](int i) { return std::to_string(i);}; // just lifting

      THEN("positive values are mapped actually")
      {
        // TODO (usability) std::string shouldn't be written here
        // moreover string should not be created if contains Some
        REQUIRE(opt_0.map_or(stringify, std::string("a")) == "0");
        REQUIRE(opt_1.map_or(stringify, std::string("a")) == "1");
        REQUIRE(opt_n.map_or(stringify, std::string("a")) == "a");
      }
    }

    WHEN("they are iterated through")
    {
      THEN("they are ok")
      {
        COMPARE_RANGES(opt_0, (std::initializer_list<int>{0}));
        COMPARE_RANGES(opt_1, (std::initializer_list<int>{1}));
        COMPARE_RANGES(opt_n, (std::initializer_list<int>{}));
      }
    }

  }
}

struct move_only
{
  int i;
  explicit move_only(int i) : i(i) {}

  move_only(const move_only&) = delete;
  move_only& operator=(const move_only&) = delete;

  move_only(move_only&&) = default;
  move_only& operator=(move_only&&) = default;
};

SCENARIO("visitable feature")
{
  GIVEN("a variant strong type which is visitable")
  {
    using V = strong_type<std::variant<int, std::string>, struct Tag, visitable>;
    // Should I open std namespace to support it?
    // WHEN("std::visit called on it")
    // {
    //   REQUIRE(std::visit(ezy::overloaded{
    //       [](int) -> std::string { return "int"; },
    //       [](const std::string&) -> std::string { return "string"; }
    //       }, v)== "string");
    // }
    THEN("visit can be called on a mutable instance")
    {
      V v{"foobar"};
      REQUIRE(v.visit(
          [](int) -> std::string { return "int"; },
          [](const std::string&) -> std::string { return "string"; }
          ) == "string");
    }

    THEN("visit can be called on an immutable instance")
    {
      const V v{"foobar"};
      REQUIRE(v.visit(
          [](int) -> std::string { return "int"; },
          [](const std::string&) -> std::string { return "string"; }
          ) == "string");
    }

    THEN("visit can be called on a prvalue instance")
    {
      using VM = strong_type<std::variant<move_only, std::string>, struct Tag, visitable>;

      move_only m{3};
      REQUIRE(VM{move_only(10)}.visit(
          [&](move_only&& mo) -> std::string { m = std::move(mo); return "move-only"; },
          [](std::string&&) -> std::string { return "string"; }
          ) == "move-only");

      REQUIRE(m.i == 10);
    }
  }
}


SCENARIO("result-like continuation")
{

  auto twice = [](int i) {return i*2;};
  GIVEN("is_success")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    REQUIRE(R{10}.is_success());
    REQUIRE(!R{"alma"}.is_success());
  }

  GIVEN("is_error")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    REQUIRE(!R{10}.is_error());
    REQUIRE(R{"alma"}.is_error());
  }

  GIVEN("success_or")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    REQUIRE(R{10}.success_or(3) == 10);
    REQUIRE(R{"alma"}.success_or(3) == 3);
  }

  GIVEN("success_or -- const")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    const R r1{10};
    REQUIRE(r1.success_or(3) == 10);

    const R r2{"alma"};
    REQUIRE(r2.success_or(3) == 3);
  }

  GIVEN("success_or -- moves") {
    using R = strong_type<std::variant<move_only, std::string>, void, result_like_continuation>;
    REQUIRE(R{move_only{5}}.success_or(move_only{4}).i == 5);
    REQUIRE(R{"alma"}.success_or(move_only{4}).i == 4);
  }

  GIVEN("map")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    REQUIRE(std::get<int>(R{10}.map(twice).map(twice).get()) == 40);
    REQUIRE(std::get<std::string>(R{"hoo"}.map(twice).map(twice).get()) == "hoo");
  }

  GIVEN("map -- same types")
  {
    using R = strong_type<std::variant<int, int>, void, result_like_continuation>;
    using V = typename get_underlying_type<R>::type;
    REQUIRE(std::get<0>(R{V(std::in_place_index_t<0>{}, 10)}.map(twice).map(twice).get()) == 40);
    REQUIRE(std::get<1>(R{V(std::in_place_index_t<1>{}, 15)}.map(twice).map(twice).get()) == 15);
  }
  GIVEN("map -- changing type")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    auto and_a_half = [](int i) {return i*1.5;};
    REQUIRE(std::get<double>(R{10}.map(and_a_half).get()) == 15.0);
    REQUIRE(std::get<std::string>(R{"hoo"}.map(and_a_half).get()) == "hoo");
  }

  GIVEN("map -- properly moves rvalue as a success type")
  {
    using R = strong_type<std::variant<move_only, int>, void, result_like_continuation>;
    move_only result{4};
    REQUIRE(std::get<double>(R{move_only{10}}.map([&](move_only&& m) { result = std::move(m); return 2.0;}).get()) == 2.0);
    REQUIRE(result.i == 10);
  }

  GIVEN("map -- properly moves rvalue as an error type")
  {
    using R = strong_type<std::variant<int, move_only>, void, result_like_continuation>;
    REQUIRE(std::get<move_only>(R{move_only{10}}.map([](int i) { return 2.0;}).get()).i == 10);
  }

  GIVEN("map -- const")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    const R r{10};
    REQUIRE(std::get<int>(r.map(twice).get()) == 20);
  }

  GIVEN("map_or")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    REQUIRE(R{10}.map_or(twice, 2) == 20);
    REQUIRE(R{"hoo"}.map_or(twice, 2) == 2);
  }

  // TODO underlying type is const?
  GIVEN("map_or -- const")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    const R r1{10};
    REQUIRE(r1.map_or(twice, 2) == 20);
    const R r2{"hoo"};
    REQUIRE(r2.map_or(twice, 2) == 2);
  }

  GIVEN("map_or -- move")
  {
    using R = strong_type<std::variant<move_only, std::string>, void, result_like_continuation>;
    REQUIRE(R{move_only{4}}.map_or([](move_only&& m){ return move_only{123};}, move_only{9}).i == 123);
    REQUIRE(R{"hoo"}.map_or([](move_only&& m){ return move_only{123};}, move_only{9}).i == 9);
  }

  GIVEN("and_then")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    auto half = [](int i) -> R {if (i % 2 == 0) return R{i / 2}; else return R{"oops"};};
    REQUIRE(std::get<int>(R{10}.and_then(half).get()) == 5);
    REQUIRE(std::get<std::string>(R{10}.and_then(half).and_then(half).get()) == "oops");
  }

  GIVEN("and_then -- same types")
  {
    using R = strong_type<std::variant<int, int>, void, result_like_continuation>;
    auto half = [](int i) -> R {
      if (i % 2 == 0)
        return R{std::in_place_index_t<0>{}, i / 2}; // TODO helper in strong type. eg R::make_success(i / 2)
      else
        return R{std::in_place_index_t<1>{}, 100};
    };
    using V = typename get_underlying_type<R>::type;
    REQUIRE(std::get<1>(R{V(std::in_place_index_t<0>{}, 10)}.and_then(half).and_then(half).get()) == 100);
    REQUIRE(std::get<0>(R{V(std::in_place_index_t<0>{}, 64)}.and_then(half).and_then(half).get()) == 16);
    REQUIRE(std::get<1>(R{V(std::in_place_index_t<1>{}, 64)}.and_then(half).and_then(half).get()) == 64);
  }

  GIVEN("and_then -- changing type")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    using R2 = rebind_strong_type_t<R, std::variant<double, std::string>>;
    auto change = [](int i) -> R2 {if (i % 3 == 0) return R2{i / 3}; else return R2{"oops"};};
    REQUIRE(std::get<double>(R{9}.and_then(change).get()) == 3.0);
    REQUIRE(std::get<double>(R{9}.and_then(change).and_then(change).get()) == 1.0);
  }

  GIVEN("and_then -- changing type")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    using R2 = rebind_strong_type_t<R, std::variant<double, std::string>>;
    auto change = [](int i) -> R2 {if (i % 3 == 0) return R2{i / 3}; else return R2{"oops"};};
    REQUIRE(std::get<double>(R{9}.and_then(change).get()) == 3.0);
    REQUIRE(std::get<double>(R{9}.and_then(change).and_then(change).get()) == 1.0);
    REQUIRE(std::get<std::string>(R{9}.and_then(change).and_then(change).and_then(change).get()) == "oops");
  }
  GIVEN("and_then -- function returning underlying type")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    using V = std::variant<int, std::string>;
    auto half = [](int i) -> V {if (i % 2 == 0) return V{i / 2}; else return V{"oops"};};
    REQUIRE(std::get<int>(R{10}.and_then(half).get()) == 5);
    REQUIRE(std::get<std::string>(R{10}.and_then(half).and_then(half).get()) == "oops");
  }
  GIVEN("and_then -- properly moves rvalue")
  {
    using V = strong_type<std::variant<move_only, int>, void, result_like_continuation>;
    move_only result{1};
    REQUIRE(std::get<int>(V{move_only{10}}.and_then([&](move_only&&m) -> V { result = std::move(m); return V{5};}).get()) == 5);
    REQUIRE(result.i == 10);
  }

  GIVEN("and_then -- properly moves rvalue as an error type")
  {
    using R = strong_type<std::variant<int, move_only>, void, result_like_continuation>;
    REQUIRE(std::get<move_only>(R{move_only{10}}.and_then([](int i) { return R{move_only{3}};}).get()).i == 10);
  }

  GIVEN("and_then -- const")
  {
    using R = strong_type<std::variant<int, std::string>, void, result_like_continuation>;
    const R r{10};
    REQUIRE(std::get<int>(r.and_then([](int i){ return R{i + 1};}).get()) == 11);
  }
}

SCENARIO("result like interface for std::optional")
{
  auto twice = [](int i) {return i*2;};

  using O = strong_type<std::optional<int>, void, result_interface<optional_adapter>::continuation>;
  auto half = [](int i) -> O {if (i % 2 == 0) return O{i / 2}; else return O{std::nullopt};};
  GIVEN("map")
  {
    REQUIRE(O{10}.map(twice).map(twice).get().value() == 40);
    REQUIRE(!O{std::nullopt}.map(twice).map(twice).get().has_value());
  }

  GIVEN("map -- changing type")
  {
    auto and_a_half = [](int i) {return i*1.5;};
    REQUIRE(O{10}.map(and_a_half).get().value() == 15.0);
    REQUIRE(!O{std::nullopt}.map(and_a_half).map(twice).get().has_value());
  }

  GIVEN("map -- properly moves")
  {
    using Om = strong_type<std::optional<move_only>, void, result_interface<optional_adapter>::continuation>;
    move_only result{4};
    REQUIRE(Om{move_only{10}}.map([&](move_only&& m) { result = std::move(m); return 2.0;}).get().value() == 2.0);
    REQUIRE(result.i == 10);
  }

  GIVEN("and_then")
  {
    REQUIRE(!O{10}.and_then(half).and_then(half).get().has_value());
    REQUIRE(O{20}.and_then(half).and_then(half).get().value() == 5);
    REQUIRE(!O{std::nullopt}.and_then(half).and_then(half).get().has_value());
  }

  GIVEN("and_then -- changing type")
  {
    using O2 = rebind_strong_type_t<O, std::optional<double>>;
    auto change = [](int i) -> O2 {if (i % 3 == 0) return O2{i / 3}; else return O2{std::nullopt};};
    REQUIRE(O{9}.and_then(change).get().value() == 3.0);
    REQUIRE(O{9}.and_then(change).and_then(change).get().value() == 1.0);
    REQUIRE(!O{9}.and_then(change).and_then(change).and_then(change).get().has_value());
    REQUIRE(!O{10}.and_then(change).get().has_value());
  }

  GIVEN("and_then -- properly moves")
  {
    using Om = strong_type<std::optional<move_only>, void, result_interface<optional_adapter>::continuation>;
    move_only result{4};
    REQUIRE(Om{move_only{10}}.and_then([&](move_only&& m) { result = std::move(m); return Om{2};}).get().value().i == 2);
    REQUIRE(result.i == 10);
  }

}

template <typename ... Ts>
using variant = Enumeration<Ts...>;

struct Field
{
  std::string name;
  std::string value;
};

struct Valid {};
struct Invalid {};
using ValidField = strong_type<Field, Valid>;
using InvalidField = strong_type<Field, Invalid>;

const auto fieldValidity = ezy::overloaded{
  [](const ValidField&) -> std::string { return "valid"; },
  [](const InvalidField&) -> std::string { return "invalid"; }
};


template <template <typename...> class VariantType>
void variant_feature_parity()
{
  using Fld = VariantType<ValidField, InvalidField>;

  Fld f = ValidField{"asd", "bsd"};


  /*
  std::visit(ezy::overloaded{
      [](const ValidField& f) {std::cout << "valid"},
      [](const InvalidField& f) {std::cout << "invalid"}
      }, std::static_cast<std::variant<ValidField, InvalidField>>(f));
      */
}


SCENARIO("learning tests")
{
  variant_feature_parity<std::variant>();
  variant_feature_parity<Enumeration>();

  using Fld = Enumeration<ValidField, InvalidField>;
  Fld valid = ValidField{"name", "xxx"};
  Fld invalid = InvalidField{"age", "thirty"};

  REQUIRE(valid.match(fieldValidity) == "valid");
  REQUIRE(invalid.match(fieldValidity) == "invalid");
}

SCENARIO("result type")
{
  // TODO
}

/**
 * strong type general
 */

SCENARIO("strong type for integer")
{
  using ST = strong_type<int, struct DummyTag>;
  WHEN("Constructed")
  {
    const ST st(12);
    THEN("underlying type can got")
    {
      REQUIRE(st.get() == 12);
    }
    THEN("static cast works")
    {
      REQUIRE(static_cast<int>(st) == 12);
    }
  }

  WHEN("constructed from non-const lvalue")
  {
    int u = 3;
    const ST st(u);
    THEN("underlying type can got")
    {
      REQUIRE(st.get() == 3);
    }
  }

  WHEN("Copy constructed")
  {
    const ST st(123);
    const ST other(st);
    THEN("its value is also copied")
    {
      REQUIRE(st.get() == 123);
      REQUIRE(other.get() == 123);
    }
  }

  WHEN("Move constructed")
  {
    ST st(123);
    const ST other(std::move(st));
    THEN("its value is also copied")
    {
      REQUIRE(other.get() == 123);
    }
  }

  // TODO (feature) features -> callable: inherits operator()
}

SCENARIO("strong type for struct")
{
  struct MyStruct
  {
    int i;
    std::string s;
  };

  using ST = strong_type<MyStruct, struct DummyTag>;

  WHEN("constructed")
  {
    const ST st{3, "str"};
    THEN("underlying values can got back")
    {
      REQUIRE(st.get().i == 3);
      REQUIRE(st.get().s == "str");
    }
  }

  WHEN("constructed from non-const lvalue")
  {
    MyStruct u{4, "str"};
    const ST st(u);
    THEN("underlying type can got")
    {
      REQUIRE(st.get().i == 4);
      REQUIRE(st.get().s == "str");
    }
  }


  WHEN("copy constructed")
  {
    const ST st{3, "str"};
    const ST other(st);
    THEN("its values are properly copied")
    {
      REQUIRE(other.get().i == 3);
      REQUIRE(other.get().s == "str");
    }
  }

  WHEN("move constructed")
  {
    ST st{4, "str"};
    const ST other(std::move(st));
    THEN("its values are also moved")
    {
      REQUIRE(other.get().i == 4);
      REQUIRE(other.get().s == "str");
    }
  }
}

SCENARIO("strong type for const struct")
{
  struct MyStruct
  {
    int i;
    std::string s;
  };

  using ST = strong_type<const MyStruct, struct DummyTag>;

  WHEN("constructed")
  {
    const ST st{3, "str"};
    THEN("underlying values can got back")
    {
      REQUIRE(st.get().i == 3);
      REQUIRE(st.get().s == "str");
    }
  }

  WHEN("constructed from non-const lvalue")
  {
    MyStruct u{4, "str"};
    const ST st(u);
    THEN("underlying type can got")
    {
      REQUIRE(st.get().i == 4);
      REQUIRE(st.get().s == "str");
    }
  }


  WHEN("copy constructed")
  {
    const ST st{3, "str"};
    const ST other(st);
    THEN("its values are properly copied")
    {
      REQUIRE(other.get().i == 3);
      REQUIRE(other.get().s == "str");
    }
  }

  WHEN("move constructed")
  {
    ST st{4, "str"};
    const ST other(std::move(st));
    THEN("its values are also moved")
    {
      REQUIRE(other.get().i == 4);
      REQUIRE(other.get().s == "str");
    }
  }
}

SCENARIO("strong type reference for struct")
{
  struct MyStruct
  {
    int i;
    std::string s;
  };

  using ST = strong_type_reference<MyStruct, struct DummyTag>;
  MyStruct s{4, "str"};

  WHEN("constructed")
  {
    const ST st(s);
    THEN("underlying values can got back")
    {
      REQUIRE(st.get().i == 4);
      REQUIRE(st.get().s == "str");
    }
  }

  WHEN("referred instance changed")
  {
    const ST st(s);
    s.i = 5;
    s.s = "asd";
    THEN("underlying values can got back")
    {
      REQUIRE(st.get().i == 5);
      REQUIRE(st.get().s == "asd");
    }
  }

  WHEN("copy constructed")
  {
    const ST st(s);
    const ST other(st);
    THEN("its values are properly copied")
    {
      REQUIRE(other.get().i == 4);
      REQUIRE(other.get().s == "str");
    }
  }

  WHEN("move constructed")
  {
    static_assert(std::is_move_constructible_v<ST>);
    ST st(s);
    const ST other(std::move(st));
    THEN("its values are also moved")
    {
      REQUIRE(other.get().i == 4);
      REQUIRE(other.get().s == "str");
    }
  }
}

SCENARIO("strong type for vector")
{
  using ST = strong_type<std::vector<int>, struct DummyTag>;

  WHEN("constructed with init list")
  {
    const ST st{4, 5, 7, 9};
    THEN("it contains all the values")
    {
      REQUIRE(st.get().size() == 4);
      REQUIRE(st.get().at(0) == 4);
      REQUIRE(st.get().at(1) == 5);
      REQUIRE(st.get().at(2) == 7);
      REQUIRE(st.get().at(3) == 9);
    }
  }

  WHEN("copy constructed")
  {
    const ST st{4, 5, 7, 9};
    const ST other(st);
    THEN("it contains all the values")
    {
      REQUIRE(other.get().size() == 4);
      REQUIRE(other.get().at(0) == 4);
      REQUIRE(other.get().at(1) == 5);
      REQUIRE(other.get().at(2) == 7);
      REQUIRE(other.get().at(3) == 9);
    }
  }

  WHEN("move constructed")
  {
    ST st{4, 5, 7, 9};
    const ST other(std::move(st));
    THEN("it contains all the values")
    {
      REQUIRE(other.get().size() == 4);
      REQUIRE(other.get().at(0) == 4);
      REQUIRE(other.get().at(1) == 5);
      REQUIRE(other.get().at(2) == 7);
      REQUIRE(other.get().at(3) == 9);
    }
  }
}

SCENARIO("strong type for const integer")
{
  using ST = strong_type<const int, struct DummyTag>;
  WHEN("constructed")
  {
    const ST st(123);
    THEN("underlying type can be got back")
    {
      REQUIRE(st.get() == 123);
    }
  }

  WHEN("constructed from non-const lvalue")
  {
    int u = 3;
    const ST st(u);
    THEN("underlying type can got")
    {
      REQUIRE(st.get() == 3);
    }
  }

  WHEN("copy constructed")
  {
    const ST st(123);
    const ST other(st);
    THEN("underlying type can be got back")
    {
      REQUIRE(st.get() == 123);
    }
  }
}

SCENARIO("strong type constructions")
{
  WHEN("underlying type is brace constructible")
  {
    struct S {int i; double d;};
    THEN("strong type can construct it")
    {
      using ST = strong_type<S, struct Tag>;
      ST st{1, 1.2};
      REQUIRE(st.get().i == 1);
      REQUIRE(st.get().d == 1.2);
    }
  }

  WHEN("underlying type is noexplicit not brace constructible")
  {
    struct S { S(double p) : i(p), d(p*2) {} int i; double d;};
    THEN("strong type can construct it")
    {
      using ST = strong_type<S, struct Tag>;
      ST st{1.2};
      REQUIRE(st.get().i == 1);
      REQUIRE(st.get().d == 2.4);
    }
  }

  WHEN("underlying type is noexplicit binary not brace constructible")
  {
    struct S { S(double p1, double p2) : i(p1 + p2), d(p1*2 - p2) {} int i; double d;};
    THEN("strong type can construct it")
    {
      using ST = strong_type<S, struct Tag>;
      ST st{2.3, 4.1};
      REQUIRE(st.get().i == 6);
      REQUIRE(st.get().d == 0.5);
    }
  }

  WHEN("underlying type is explicit binary not brace constructible")
  {
    struct S { explicit S(double p1, double p2) : i(p1 + p2), d(p1*2 - p2) {} int i; double d;};
    THEN("strong type can construct it")
    {
      using ST = strong_type<S, struct Tag>;
      ST st{2.3, 4.1};
      REQUIRE(st.get().i == 6);
      REQUIRE(st.get().d == 0.5);
    }
  }
  
}

/**
 * strong type features: arithmetic features
 */
SCENARIO("strong type integer arithmetic")
{
  WHEN("addable")
  {
    using Add = strong_type<int, struct Tag, addable>;
    THEN("addition works")
    {
      Add a(5);
      Add b(3);
      static_assert(std::is_same_v<Add, decltype(a + b)>);
      REQUIRE((a + b).get() == 8);
    }

    THEN("additive update works")
    {
      Add a(5);
      a += Add(4);
      REQUIRE(a.get() == 9);
    }
  }

  WHEN("subtractable")
  {
    using Sub = strong_type<int, struct Tag, subtractable>;
    THEN("subtraction works")
    {
      Sub a(5);
      Sub b(3);
      static_assert(std::is_same_v<Sub, decltype(a - b)>);
      REQUIRE((a - b).get() == 2);
    }

    THEN("subtractive update works")
    {
      Sub a(5);
      a -= Sub(4);
      REQUIRE(a.get() == 1);
    }
  }
}

/**
 * strong type traits
 */
SCENARIO("compilation tests")
{
  using Simple = strong_type<int, struct Tag>;
  using SimpleRef = strong_type_reference<int, struct Tag>;
  using OneFeature = strong_type<int, struct Tag, addable>;
  using MoreFeatures = strong_type<int, struct Tag, addable, subtractable, equal_comparable>;

  static_assert(is_strong_type_v<int> == false);
  static_assert(is_strong_type_v<Simple> == true);
  static_assert(is_strong_type_v<SimpleRef> == true);
  static_assert(is_strong_type_v<OneFeature> == true);
  static_assert(is_strong_type_v<MoreFeatures> == true);

  static_assert(std::is_same_v<plain_type_t<int>, int>);
  static_assert(std::is_same_v<plain_type_t<Simple>, int>);
  static_assert(std::is_same_v<plain_type_t<SimpleRef>, int&>);
  static_assert(std::is_same_v<plain_type_t<OneFeature>, int>);
  static_assert(std::is_same_v<plain_type_t<MoreFeatures>, int>);

  // TODO rename: extract_underlying_type
  //static_assert(std::is_same_v<get_underlying_type<int>::type, int>); // must fail
  static_assert(std::is_same_v<get_underlying_type<Simple>::type, int>);
  static_assert(std::is_same_v<get_underlying_type<SimpleRef>::type, int&>);
  static_assert(std::is_same_v<get_underlying_type<OneFeature>::type, int>);
  static_assert(std::is_same_v<get_underlying_type<MoreFeatures>::type, int>);

  //static_assert(std::is_same_v<extract_tag_t<int>, struct Tag>); // must fail
  static_assert(std::is_same_v<extract_tag_t<Simple>, struct Tag>);
  static_assert(std::is_same_v<extract_tag_t<SimpleRef>, struct Tag>);
  static_assert(std::is_same_v<extract_tag_t<OneFeature>, struct Tag>);
  static_assert(std::is_same_v<extract_tag_t<MoreFeatures>, struct Tag>);

  static_assert(std::is_same_v<extract_features_t<Simple>, std::tuple<>>);
  static_assert(std::is_same_v<extract_features_t<SimpleRef>, std::tuple<>>);
  static_assert(std::is_same_v<extract_features_t<OneFeature>, std::tuple<addable<OneFeature>>>);
  static_assert(std::is_same_v<extract_features_t<MoreFeatures>, std::tuple<addable<MoreFeatures>, subtractable<MoreFeatures>, equal_comparable<MoreFeatures>>>);

  static_assert(std::is_same_v<strip_strong_type_t<Simple>, Simple>);
  static_assert(std::is_same_v<strip_strong_type_t<SimpleRef>, SimpleRef>);
  static_assert(std::is_same_v<strip_strong_type_t<OneFeature>, Simple>);
  static_assert(std::is_same_v<strip_strong_type_t<MoreFeatures>, Simple>);

  static_assert(std::is_same_v<rebind_strong_type_t<Simple, double>, strong_type<double, struct Tag>>);
  static_assert(std::is_same_v<rebind_strong_type_t<SimpleRef, double>, strong_type<double, struct Tag>>); // TODO think: not a strong_type_reference
  static_assert(std::is_same_v<rebind_strong_type_t<OneFeature, double>, strong_type<double, struct Tag, addable>>);
  static_assert(std::is_same_v<rebind_strong_type_t<MoreFeatures, double>, strong_type<double, struct Tag, addable, subtractable, equal_comparable>>);

  // TODO it will not work in any case (eg. derived features)
  /*
  static_assert(has_feature_v<Simple, addable> == false);
  static_assert(has_feature_v<SimpleRef, addable> == false);
  static_assert(has_feature_v<OneFeature, addable> == true);
  static_assert(has_feature_v<OneFeature, equal_comparable> == false);
  */

  // TODO feature_cast => when type and tags are the same and only features are different, casting should be allowed

  /**
   * TODO what about derivation?
   *
   * struct S : strong_type<...>
   * { ... };
   *
   */
}

#include <ezy/experimental/function>

const auto str_plus_int = [](const std::string& s, const int i)
{ return s + std::to_string(i); };

SCENARIO("curry")
{
  using namespace ezy::experimental::function;

  GIVEN("a function object")
  {
    WHEN("curried")
    {
      const auto curried = curry<int, int>(std::plus<int>{});
      const auto addFive = curried(5);
      REQUIRE(addFive(10) == 15);

    }
  }

  GIVEN("a lambda with different types")
  {
    REQUIRE(str_plus_int("a", 2) == "a2");

    WHEN("curried")
    {
      const auto curried = curry<std::string, int>(str_plus_int);
      const auto prefixed = curried("number: ");
      REQUIRE(prefixed(123) == "number: 123");
    }
  }
}

SCENARIO("compose")
{
  using namespace ezy::experimental::function;
  auto addHundred = curry<int, int>(std::plus<int>{})(100);

  WHEN("composed with same type")
  {
    auto addTwoHundred = compose(addHundred, addHundred);
    REQUIRE(addTwoHundred(23) == 223);
  }

  WHEN("composed with different type")
  {
    // TODO const lambdas are not accepted!
    auto formatNumber = curry<std::string, int>(str_plus_int)("result is: ");
    auto calculate = compose(addHundred, formatNumber);
    REQUIRE(calculate(23) == "result is: 123");
  }

  WHEN("access member")
  {
    struct S{ int i; };

    auto calculate = compose(&S::i, addHundred);

    S s{3};
    REQUIRE(calculate(s) == 103);
  }
}
