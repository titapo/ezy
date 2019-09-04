#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include "../include/ezy/strong_type"

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
  using MyNumbers = ezy::strong_type<std::vector<int>, struct MyNumbersTag, ezy::features::iterable>;
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
  using MyNumbers = ezy::strong_type<std::vector<int>, struct MyNumbersTag, ezy::features::iterable>;
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

    WHEN("finding element works with different type")
    {
      struct S
      {
        int i;
        bool operator==(int j) const { return i == j; }
      };

      const auto svec = numbers.map([](int i) {return S{i};}).to_iterable<std::vector<S>>();
      REQUIRE(svec.find(3).has_value());
      REQUIRE(!svec.find(-3).has_value());
    }

    WHEN("find existing")
    {
      const auto result = numbers.find(5);
      REQUIRE(result.has_value());
      REQUIRE(result.value_or(99) == 5);
    }

    WHEN("find non existing")
    {
      const auto result = numbers.find(15);
      REQUIRE(!result.has_value());
      REQUIRE(result.value_or(99) == 99);
    }

    WHEN("find works on map")
    {
      using StrongMap = ezy::strong_type<std::map<std::string, int>, struct notag, ezy::features::iterable>;
      StrongMap sm = StrongMap{std::map<std::string, int>{{"Alpha", 10}, {"Beta", 20}}}; // TODO this is ugly

      const auto alpha = sm.find("Alpha");
      REQUIRE(alpha.has_value());
      REQUIRE(alpha.value().first == "Alpha");
      REQUIRE(alpha.value().second == 10);

      const auto beta = sm.find("Beta");
      REQUIRE(beta.has_value());
      REQUIRE(beta.value().first == "Beta");
      REQUIRE(beta.value().second == 20);

      REQUIRE(!sm.find("Gamma").has_value());
    }

    WHEN("find_if existing")
    {
      const auto result = numbers.find_if(less_than(5));
      REQUIRE(result.has_value());
      REQUIRE(result.value_or(99) == 1);
    }

    WHEN("find_if non existing")
    {
      const auto result = numbers.find_if(less_than(0));
      REQUIRE(!result.has_value());
      REQUIRE(result.value_or(99) == 99);
    }

    WHEN("contains element")
    {
      REQUIRE(numbers.contains(3));
      REQUIRE(!numbers.contains(-3));
    }

    WHEN("contains element works with different type")
    {
      struct S
      {
        int i;
        bool operator==(int j) const { return i == j; }
      };

      const auto svec = numbers.map([](int i) {return S{i};}).to_iterable<std::vector<S>>();
      REQUIRE(svec.contains(3));
      REQUIRE(!svec.contains(-3));
    }

    WHEN("contains works on map")
    {
      using StrongMap = ezy::strong_type<std::map<std::string, int>, struct notag, ezy::features::iterable>;
      StrongMap sm = StrongMap{std::map<std::string, int>{{"Alpha", 10}, {"Beta", 20}}}; // TODO this is ugly
      REQUIRE(sm.contains("Alpha"));
      REQUIRE(sm.contains("Beta"));
      REQUIRE(!sm.contains("Gamma"));
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
   
    WHEN("flattened")
    {
      // FIXME it seems that MyNumbers has no const_iterator
      using Raised = ezy::strong_type< std::vector<std::vector<int>>, struct RaisedTag, ezy::features::iterable>;

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

    WHEN("checked for empty")
    {
      REQUIRE(MyNumbers{}.empty());
      REQUIRE(!MyNumbers{1}.empty());
      REQUIRE(!MyNumbers{1, 2}.empty());
    }

    WHEN("checked for size")
    {
      REQUIRE(MyNumbers{}.size() == 0);
      REQUIRE(MyNumbers{1}.size() == 1);
      REQUIRE(MyNumbers{1, 2}.size() == 2);
      REQUIRE(numbers.size() == 10);
    }

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

  GIVEN("a special type")
  {
    struct Spec
    {
      constexpr int size() const noexcept { return 42; }
      constexpr bool empty() const noexcept { return false; }
    };

    using StrongSpec = ezy::strong_type<Spec, void, ezy::features::iterable>;

    WHEN("size is called on it")
    {
      StrongSpec s;
      REQUIRE(s.size() == 42);
    }

    WHEN("empty is called on it")
    {
      StrongSpec s;
      REQUIRE(s.empty() == false);
    }
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

// TODO think of: making ezy::optional and ezy::result iterable?

#include "../include/ezy/utility.h"

struct move_only
{
  int i;
  explicit move_only(int i) : i(i) {}

  move_only(const move_only&) = delete;
  move_only& operator=(const move_only&) = delete;

  move_only(move_only&&) = default;
  move_only& operator=(move_only&&) = default;
};

struct non_transferable
{
  int i;
  explicit non_transferable(int i) : i(i) {}

  non_transferable(const non_transferable&) = delete;
  non_transferable& operator=(const non_transferable&) = delete;

  non_transferable(non_transferable&&) = delete;
  non_transferable& operator=(non_transferable&&) = delete;
};


SCENARIO("feature: inherit_std_optional")
{
  GIVEN("a strong type with feature")
  {
    struct S {int i;};
    using Opt = ezy::strong_type<std::optional<S>, void, ezy::features::inherit_std_optional>;
    using OptMo = ezy::strong_type<std::optional<move_only>, void, ezy::features::inherit_std_optional>;
    WHEN("an instance holds a value")
    {
      Opt o{S{42}};
      THEN("arrow operator works")
      {
        REQUIRE(o->i == 42);
        o->i = 10;
        REQUIRE(o->i == 10);
      }

      THEN("star operator works")
      {
        REQUIRE((*o).i == 42);
        (*o).i = 11;
        REQUIRE((*o).i == 11);
      }

      THEN("it's convertible to bool")
      {
        REQUIRE(!!o);
      }
      THEN("has_value returns")
      {
        REQUIRE(o.has_value());
      }

      THEN("value returned")
      {
        REQUIRE(o.value().i == 42);
      }
      THEN("value_or() returned")
      {
        REQUIRE(o.value_or(S{15}).i == 42);
      }
    }

    WHEN("star operator called on prvalue")
    THEN("it works")
    {
      move_only result = *OptMo{move_only{42}};
      REQUIRE(result.i == 42);
    }

    WHEN("star operator called on moved object")
    THEN("it works")
    {
      auto mo = OptMo{move_only{42}};
      move_only result = *std::move(mo);
      REQUIRE(result.i == 42);
    }

    WHEN("an instance holds nothing")
    {
      Opt o{std::nullopt};
      // THEN("calling star or arrow operator results undefined behaviour")
      THEN("it's convertible to bool")
      {
        REQUIRE(!o);
      }
      THEN("has_value returns")
      {
        REQUIRE(!o.has_value());
      }

      THEN("value() throws")
      {
        REQUIRE_THROWS_AS(o.value(), std::bad_optional_access);
      }

      THEN("value_or() returns default")
      {
        REQUIRE(o.value_or(S{15}).i == 15);
      }
    }
  }
}

SCENARIO("visitable feature")
{
  GIVEN("a variant strong type which is visitable")
  {
    using V = ezy::strong_type<std::variant<int, std::string>, struct Tag, ezy::features::visitable>;
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
      using VM = ezy::strong_type<std::variant<move_only, std::string>, struct Tag, ezy::features::visitable>;

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
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    REQUIRE(R{10}.is_success());
    REQUIRE(!R{"alma"}.is_success());
  }

  GIVEN("is_error")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    REQUIRE(!R{10}.is_error());
    REQUIRE(R{"alma"}.is_error());
  }

  /*
  GIVEN("ctor works with non_transferable")
  {
    using R = ezy::strong_type<std::variant<non_transferable, std::string>, void, ezy::features::result_like_continuation>;
    using V = ezy::extract_underlying_type_t<R>;
    REQUIRE(!R{V(std::in_place_index_t<0>{}, 10)}.is_error());
  }
  //TODO GIVEN("make_success works with non_transferable")
  //TODO GIVEN("make_error works with non_transferable")
  */

  GIVEN("success_or")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    REQUIRE(R{10}.success_or(3) == 10);
    REQUIRE(R{"alma"}.success_or(3) == 3);
  }

  GIVEN("success_or -- const")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    const R r1{10};
    REQUIRE(r1.success_or(3) == 10);

    const R r2{"alma"};
    REQUIRE(r2.success_or(3) == 3);
  }

  GIVEN("success_or -- moves") {
    using R = ezy::strong_type<std::variant<move_only, std::string>, void, ezy::features::result_like_continuation>;
    REQUIRE(R{move_only{5}}.success_or(move_only{4}).i == 5);
    REQUIRE(R{"alma"}.success_or(move_only{4}).i == 4);
  }

  GIVEN("map")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    REQUIRE(std::get<int>(R{10}.map(twice).map(twice).get()) == 40);
    REQUIRE(std::get<std::string>(R{"hoo"}.map(twice).map(twice).get()) == "hoo");
  }

  GIVEN("map -- same types")
  {
    using R = ezy::strong_type<std::variant<int, int>, void, ezy::features::result_like_continuation>;
    using V = ezy::extract_underlying_type_t<R>;
    REQUIRE(std::get<0>(R::make_success(10).map(twice).map(twice).get()) == 40);
    REQUIRE(std::get<1>(R::make_error(15).map(twice).map(twice).get()) == 15);
  }
  GIVEN("map -- changing type")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    auto and_a_half = [](int i) {return i*1.5;};
    REQUIRE(std::get<double>(R{10}.map(and_a_half).get()) == 15.0);
    REQUIRE(std::get<std::string>(R{"hoo"}.map(and_a_half).get()) == "hoo");
  }

  GIVEN("map -- properly moves rvalue as a success type")
  {
    using R = ezy::strong_type<std::variant<move_only, int>, void, ezy::features::result_like_continuation>;
    move_only result{4};
    REQUIRE(std::get<double>(R{move_only{10}}.map([&](move_only&& m) { result = std::move(m); return 2.0;}).get()) == 2.0);
    REQUIRE(result.i == 10);
  }

  GIVEN("map -- properly moves rvalue as an error type")
  {
    using R = ezy::strong_type<std::variant<int, move_only>, void, ezy::features::result_like_continuation>;
    REQUIRE(std::get<move_only>(R{move_only{10}}.map([](int i) { return 2.0;}).get()).i == 10);
  }

  GIVEN("map -- const")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    const R r{10};
    REQUIRE(std::get<int>(r.map(twice).get()) == 20);
  }

  GIVEN("map_or")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    REQUIRE(R{10}.map_or(twice, 2) == 20);
    REQUIRE(R{"hoo"}.map_or(twice, 2) == 2);
  }

  // TODO underlying type is const?
  GIVEN("map_or -- const")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    const R r1{10};
    REQUIRE(r1.map_or(twice, 2) == 20);
    const R r2{"hoo"};
    REQUIRE(r2.map_or(twice, 2) == 2);
  }
  //

  GIVEN("map_or -- move")
  {
    using R = ezy::strong_type<std::variant<move_only, std::string>, void, ezy::features::result_like_continuation>;
    REQUIRE(R{move_only{4}}.map_or([](move_only&& m){ return move_only{123};}, move_only{9}).i == 123);
    REQUIRE(R{"hoo"}.map_or([](move_only&& m){ return move_only{123};}, move_only{9}).i == 9);
  }

  auto size_as_int = [](const auto& a) -> int { return a.size(); };
  GIVEN("map_or_else")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    REQUIRE(R{5}.map_or_else(twice, size_as_int) == 10);
    REQUIRE(R{"foo"}.map_or_else(twice, size_as_int) == 3);
  }

  GIVEN("map_or_else -- const")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    const R r1{5};
    REQUIRE(r1.map_or_else(twice, size_as_int) == 10);
    const R r2{"foo"};
    REQUIRE(r2.map_or_else(twice, size_as_int) == 3);
  }

  GIVEN("map_or_else -- moving success rvalue")
  {
    using R = ezy::strong_type<std::variant<move_only, std::string>, void, ezy::features::result_like_continuation>;
    move_only res{9};
    auto move_out = [&](move_only&& m) {res = std::move(m); return res.i + 2; };
    REQUIRE(R{move_only{5}}.map_or_else(move_out, size_as_int) == 7);
    REQUIRE(res.i == 5);
    REQUIRE(R{"foo"}.map_or_else(move_out, size_as_int) == 3);
  }

  GIVEN("map_or_else -- moving error rvalue")
  {
    using R = ezy::strong_type<std::variant<int, move_only>, void, ezy::features::result_like_continuation>;
    move_only res{9};
    auto move_out = [&](move_only&& m) {res = std::move(m); return res.i + 2; };
    REQUIRE(R{5}.map_or_else(twice, move_out) == 10);
    REQUIRE(res.i == 9);
    REQUIRE(R{move_only{3}}.map_or_else(twice, move_out) == 5);
    REQUIRE(res.i == 3);
  }

  GIVEN("map_or_else<>")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    REQUIRE(R{5}.map_or_else<int>(twice, &std::string::size) == 10);
    REQUIRE(R{"foo"}.map_or_else<int>(twice, &std::string::size) == 3);
  }

  GIVEN("map_or_else<> -- const")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    const R r1{5};
    REQUIRE(r1.map_or_else<int>(twice, &std::string::size) == 10);
    const R r2{"foo"};
    REQUIRE(r2.map_or_else<int>(twice, &std::string::size) == 3);
  }

  GIVEN("map_or_else<> -- moving success rvalue")
  {
    using R = ezy::strong_type<std::variant<move_only, std::string>, void, ezy::features::result_like_continuation>;
    move_only res{9};
    auto move_out = [&](move_only&& m) {res = std::move(m); return res.i + 2; };
    REQUIRE(R{move_only{5}}.map_or_else<int>(move_out, size_as_int) == 7);
    REQUIRE(res.i == 5);
    REQUIRE(R{"foo"}.map_or_else<int>(move_out, size_as_int) == 3);
  }

  GIVEN("map_or_else<> -- moving error rvalue")
  {
    using R = ezy::strong_type<std::variant<int, move_only>, void, ezy::features::result_like_continuation>;
    move_only res{9};
    auto move_out = [&](move_only&& m) {res = std::move(m); return res.i + 2; };
    REQUIRE(R{5}.map_or_else<int>(twice, move_out) == 10);
    REQUIRE(res.i == 9);
    REQUIRE(R{move_only{3}}.map_or_else<int>(twice, move_out) == 5);
    REQUIRE(res.i == 3);
  }

  GIVEN("and_then")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    auto half = [](int i) -> R {if (i % 2 == 0) return R{i / 2}; else return R{"oops"};};
    REQUIRE(std::get<int>(R{10}.and_then(half).get()) == 5);
    REQUIRE(std::get<std::string>(R{10}.and_then(half).and_then(half).get()) == "oops");
  }

  GIVEN("and_then -- same types")
  {
    using R = ezy::strong_type<std::variant<int, int>, void, ezy::features::result_like_continuation>;
    auto half = [](int i) -> R {
      if (i % 2 == 0)
        return R::make_success(i / 2);
      else
        return R::make_error(100);
    };
    REQUIRE(std::get<1>(R::make_success(10).and_then(half).and_then(half).get()) == 100);
    REQUIRE(std::get<0>(R::make_success(64).and_then(half).and_then(half).get()) == 16);
    REQUIRE(std::get<1>(R::make_error(64).and_then(half).and_then(half).get()) == 64);
  }

  GIVEN("and_then -- changing type")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    using R2 = ezy::rebind_strong_type_t<R, std::variant<double, std::string>>;
    auto change = [](int i) -> R2 {if (i % 3 == 0) return R2{i / 3}; else return R2{"oops"};};
    REQUIRE(std::get<double>(R{9}.and_then(change).get()) == 3.0);
    REQUIRE(std::get<double>(R{9}.and_then(change).and_then(change).get()) == 1.0);
  }

  GIVEN("and_then -- changing type")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    using R2 = ezy::rebind_strong_type_t<R, std::variant<double, std::string>>;
    auto change = [](int i) -> R2 {if (i % 3 == 0) return R2{i / 3}; else return R2{"oops"};};
    REQUIRE(std::get<double>(R{9}.and_then(change).get()) == 3.0);
    REQUIRE(std::get<double>(R{9}.and_then(change).and_then(change).get()) == 1.0);
    REQUIRE(std::get<std::string>(R{9}.and_then(change).and_then(change).and_then(change).get()) == "oops");
  }
  GIVEN("and_then -- function returning underlying type")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    using V = std::variant<int, std::string>;
    auto half = [](int i) -> V {if (i % 2 == 0) return V{i / 2}; else return V{"oops"};};
    REQUIRE(std::get<int>(R{10}.and_then(half).get()) == 5);
    REQUIRE(std::get<std::string>(R{10}.and_then(half).and_then(half).get()) == "oops");
  }
  GIVEN("and_then -- properly moves rvalue")
  {
    using V = ezy::strong_type<std::variant<move_only, int>, void, ezy::features::result_like_continuation>;
    move_only result{1};
    REQUIRE(std::get<int>(V{move_only{10}}.and_then([&](move_only&&m) -> V { result = std::move(m); return V{5};}).get()) == 5);
    REQUIRE(result.i == 10);
  }

  GIVEN("and_then -- properly moves rvalue as an error type")
  {
    using R = ezy::strong_type<std::variant<int, move_only>, void, ezy::features::result_like_continuation>;
    REQUIRE(std::get<move_only>(R{move_only{10}}.and_then([](int i) { return R{move_only{3}};}).get()).i == 10);
  }

  GIVEN("and_then -- const")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    const R r{10};
    REQUIRE(std::get<int>(r.and_then([](int i){ return R{i + 1};}).get()) == 11);
  }
}

SCENARIO("result like interface for std::optional")
{
  auto twice = [](int i) {return i*2;};

  using O = ezy::strong_type<std::optional<int>, void, ezy::features::result_interface<ezy::features::optional_adapter>::continuation>;
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
    using Om = ezy::strong_type<std::optional<move_only>, void, ezy::features::result_interface<ezy::features::optional_adapter>::continuation>;
    move_only result{4};
    REQUIRE(Om{move_only{10}}.map([&](move_only&& m) { result = std::move(m); return 2.0;}).get().value() == 2.0);
    REQUIRE(result.i == 10);
  }

  WHEN("map_or called on it")
  THEN("string not need to be passed on parameter side") // not created internally
  {
using O = ezy::strong_type<std::optional<int>, void, ezy::features::result_interface<ezy::features::optional_adapter>::continuation>;
    const auto to_string = [](auto i) { return std::to_string(i); };
    auto result = O{std::nullopt}.map_or(to_string, "foo");
    static_assert(std::is_same_v<decltype(result), std::string>);
    REQUIRE(result == "foo");
  }

  GIVEN("map_or_else")
  {
    REQUIRE(O{10}.map_or_else(twice, [](std::nullopt_t){ return 4; }) == 20);
    REQUIRE(O{std::nullopt}.map_or_else(twice, [](std::nullopt_t){ return 4; }) == 4);
  }

  GIVEN("and_then")
  {
    REQUIRE(!O{10}.and_then(half).and_then(half).get().has_value());
    REQUIRE(O{20}.and_then(half).and_then(half).get().value() == 5);
    REQUIRE(!O{std::nullopt}.and_then(half).and_then(half).get().has_value());
  }

  GIVEN("and_then -- changing type")
  {
    using O2 = ezy::rebind_strong_type_t<O, std::optional<double>>;
    auto change = [](int i) -> O2 {if (i % 3 == 0) return O2{i / 3}; else return O2{std::nullopt};};
    REQUIRE(O{9}.and_then(change).get().value() == 3.0);
    REQUIRE(O{9}.and_then(change).and_then(change).get().value() == 1.0);
    REQUIRE(!O{9}.and_then(change).and_then(change).and_then(change).get().has_value());
    REQUIRE(!O{10}.and_then(change).get().has_value());
  }

  GIVEN("and_then -- properly moves")
  {
    using Om = ezy::strong_type<std::optional<move_only>, void, ezy::features::result_interface<ezy::features::optional_adapter>::continuation>;
    move_only result{4};
    REQUIRE(Om{move_only{10}}.and_then([&](move_only&& m) { result = std::move(m); return Om{2};}).get().value().i == 2);
    REQUIRE(result.i == 10);
  }

}

/**
 * strong type general
 */

SCENARIO("strong type for integer")
{
  using ST = ezy::strong_type<int, struct DummyTag>;
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

}

SCENARIO("invocable feature")
{
  struct FunctionObject
  {
    int operator()(int i, int j) const &
    { return i + j; }

    int operator()(int i, int j) &
    { return i - j; }

    int operator()(int i, int j) &&
    { return i * j; }
  };

  using CallableFnObj = ezy::strong_type<FunctionObject, struct callable, ezy::features::invocable>;
  WHEN("called")
  {
    const CallableFnObj fn;
    REQUIRE(fn(1, 2) == 3);
  }

  WHEN("called mutable")
  {
    CallableFnObj fn;
    REQUIRE(fn(1, 2) == -1);
  }

  WHEN("called rvalue")
  {
    REQUIRE(CallableFnObj{}(1, 2) == 2);
  }
}


SCENARIO("strong type for struct")
{
  struct MyStruct
  {
    int i;
    std::string s;
  };

  using ST = ezy::strong_type<MyStruct, struct DummyTag>;

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

  using ST = ezy::strong_type<const MyStruct, struct DummyTag>;

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

  using ST = ezy::strong_type_reference<MyStruct, struct DummyTag>;
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
  using ST = ezy::strong_type<std::vector<int>, struct DummyTag>;

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
  using ST = ezy::strong_type<const int, struct DummyTag>;
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
      using ST = ezy::strong_type<S, struct Tag>;
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
      using ST = ezy::strong_type<S, struct Tag>;
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
      using ST = ezy::strong_type<S, struct Tag>;
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
      using ST = ezy::strong_type<S, struct Tag>;
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
    using Add = ezy::strong_type<int, struct Tag, ezy::features::addable>;
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
    using Sub = ezy::strong_type<int, struct Tag, ezy::features::subtractable>;
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
  using Simple = ezy::strong_type<int, struct Tag>;
  using SimpleRef = ezy::strong_type_reference<int, struct Tag>;
  using OneFeature = ezy::strong_type<int, struct Tag, ezy::features::addable>;
  using MoreFeatures = ezy::strong_type<int, struct Tag, ezy::features::addable, ezy::features::subtractable, ezy::features::equal_comparable>;
  using ComposedFeature = ezy::strong_type<int, struct Tag, ezy::features::additive>;

  static_assert(ezy::is_strong_type_v<int> == false);
  static_assert(ezy::is_strong_type_v<Simple> == true);
  static_assert(ezy::is_strong_type_v<SimpleRef> == true);
  static_assert(ezy::is_strong_type_v<OneFeature> == true);
  static_assert(ezy::is_strong_type_v<MoreFeatures> == true);

  static_assert(std::is_same_v<ezy::plain_type_t<int>, int>);
  static_assert(std::is_same_v<ezy::plain_type_t<Simple>, int>);
  static_assert(std::is_same_v<ezy::plain_type_t<SimpleRef>, int&>);
  static_assert(std::is_same_v<ezy::plain_type_t<OneFeature>, int>);
  static_assert(std::is_same_v<ezy::plain_type_t<MoreFeatures>, int>);

  //static_assert(std::is_same_v<ezy::extract_underlying_type_t<int>, int>); // must fail
  static_assert(std::is_same_v<ezy::extract_underlying_type_t<Simple>, int>);
  static_assert(std::is_same_v<ezy::extract_underlying_type_t<SimpleRef>, int&>);
  static_assert(std::is_same_v<ezy::extract_underlying_type_t<OneFeature>, int>);
  static_assert(std::is_same_v<ezy::extract_underlying_type_t<MoreFeatures>, int>);

  //static_assert(std::is_same_v<extract_tag_t<int>, struct Tag>); // must fail
  static_assert(std::is_same_v<ezy::extract_tag_t<Simple>, struct Tag>);
  static_assert(std::is_same_v<ezy::extract_tag_t<SimpleRef>, struct Tag>);
  static_assert(std::is_same_v<ezy::extract_tag_t<OneFeature>, struct Tag>);
  static_assert(std::is_same_v<ezy::extract_tag_t<MoreFeatures>, struct Tag>);

  static_assert(std::is_same_v<ezy::extract_features_t<Simple>, std::tuple<>>);
  static_assert(std::is_same_v<ezy::extract_features_t<SimpleRef>, std::tuple<>>);
  static_assert(std::is_same_v<ezy::extract_features_t<OneFeature>, std::tuple<ezy::features::addable<OneFeature>>>);
  static_assert(std::is_same_v<ezy::extract_features_t<MoreFeatures>, std::tuple<ezy::features::addable<MoreFeatures>, ezy::features::subtractable<MoreFeatures>, ezy::features::equal_comparable<MoreFeatures>>>);

  static_assert(std::is_same_v<ezy::strip_strong_type_t<Simple>, Simple>);
  static_assert(std::is_same_v<ezy::strip_strong_type_t<SimpleRef>, SimpleRef>);
  static_assert(std::is_same_v<ezy::strip_strong_type_t<OneFeature>, Simple>);
  static_assert(std::is_same_v<ezy::strip_strong_type_t<MoreFeatures>, Simple>);

  static_assert(std::is_same_v<ezy::rebind_strong_type_t<Simple, double>, ezy::strong_type<double, struct Tag>>);
  static_assert(std::is_same_v<ezy::rebind_strong_type_t<SimpleRef, double>, ezy::strong_type<double, struct Tag>>); // TODO think: not a estrong_type_reference
  static_assert(std::is_same_v<ezy::rebind_strong_type_t<OneFeature, double>, ezy::strong_type<double, struct Tag, ezy::features::addable>>);
  static_assert(std::is_same_v<ezy::rebind_strong_type_t<MoreFeatures, double>, ezy::strong_type<double, struct Tag, ezy::features::addable, ezy::features::subtractable, ezy::features::equal_comparable>>);

  static_assert(ezy::has_feature_v<Simple, ezy::features::addable> == false);
  static_assert(ezy::has_feature_v<SimpleRef, ezy::features::addable> == false);
  static_assert(ezy::has_feature_v<OneFeature, ezy::features::addable> == true);
  static_assert(ezy::has_feature_v<OneFeature, ezy::features::equal_comparable> == false);
  static_assert(ezy::has_feature_v<MoreFeatures, ezy::features::equal_comparable> == true);
  // TODO it will not work in any case (eg. derived/composed features)

  static_assert(std::is_same_v<ezy::rebind_features_t<Simple>, Simple>);
  static_assert(std::is_same_v<ezy::rebind_features_t<OneFeature>, Simple>);
  static_assert(std::is_same_v<ezy::rebind_features_t<MoreFeatures>, Simple>);

  static_assert(std::is_same_v<ezy::rebind_features_t<Simple, ezy::features::addable>, OneFeature>);
  static_assert(std::is_same_v<ezy::rebind_features_t<OneFeature, ezy::features::addable>, OneFeature>);
  static_assert(std::is_same_v<ezy::rebind_features_t<MoreFeatures, ezy::features::addable>, OneFeature>);

  // TODO works with tuple
  // static_assert(std::is_same_v<ezy::rebind_features_t<Simple, ezy::extract_features_t<MoreFeatures>>, MoreFeatures>);
}




SCENARIO("derived strong type")
{

  struct DerivedSimple : ezy::strong_type<int, DerivedSimple, ezy::features::addable>
  {
    // TODO is it required?
    using strong_type::strong_type;
  };

  static_assert(!ezy::is_strong_type_v<DerivedSimple>); //it fails // should be accepted?
  static_assert(ezy::is_strong_type_v<ezy::strong_type_base_t<DerivedSimple>>);

  static_assert(std::is_same_v<ezy::strong_type_base_t<DerivedSimple>, ezy::strong_type<int, DerivedSimple, ezy::features::addable>>);
  static_assert(std::is_same_v<ezy::strong_type_base_t<ezy::strong_type_base_t<DerivedSimple>>, ezy::strong_type<int, DerivedSimple, ezy::features::addable>>);

  DerivedSimple s{3};

  REQUIRE((s + s).get() == 6);
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

#include "../include/ezy/tuple_traits"

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
