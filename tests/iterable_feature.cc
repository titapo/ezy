#include <ezy/strong_type>
#include <ezy/string.h>

#include <sstream>
#include <map>

#include <catch.hpp>

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
  for (const auto& e : range)
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

template <typename Range1, typename Range2>
constexpr bool equal(const Range1& r1, const Range2& r2)
{
  auto it1 = std::begin(r1);
  auto it2 = std::begin(r2);
  const auto end1 = std::end(r1);
  for (;it1 != end1; ++it1, ++it2)
  {
    if (!(*it1 == *it2))
      return false;
  }
  return true;
}

constexpr bool consteval_check()
{
  using Array = ezy::strong_type<std::array<int, 10>, struct ArrayTag, ezy::features::iterable>;
  Array arr{1,2,3,4,5,6,7,8,9,10};
  return equal(arr, std::array{1,2,3,4,5,6,7,8,9,10});
  //equal(arr.map([](int i) { return i + 1; }), std::array{2,3,4,5,6,7,8,9,10,11}); //TODO this should work
}

static_assert(consteval_check());

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
        COMPARE_RANGES(numbers, (std::array{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
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
        COMPARE_RANGES(result, (std::array{3, 4, 5, 6, 7, 8, 9, 10, 11, 12}));
      }
    }

    WHEN("elements mapped twice")
    {
      const auto plusTwo = [](const int& e) { return e + 2; };
      const auto &result = numbers.map(plusTwo).map(plusTwo);
      THEN ("result contains all element incremented by 4")
      {
        COMPARE_RANGES(result, (std::array{5, 6, 7, 8, 9, 10, 11, 12, 13, 14}));
      }
    }

    WHEN("elements filtered")
    {
      const auto isEven = [](const auto& e) { return e % 2 == 0; };
      const auto &result = numbers.filter(isEven);
      THEN ("result contains even numbers")
      {
        COMPARE_RANGES(result, (std::array{2, 4, 6, 8, 10}));
      }
    }

    WHEN("elements filtered twice")
    {
      const auto isEven = [](const auto& e) { return e % 2 == 0; };
      const auto isNotDivisibleBy3 = [](const auto& e) { return e % 3 != 0; };
      const auto &result = numbers.filter(isEven).filter(isNotDivisibleBy3);
      THEN ("result contains even numbers")
      {
        COMPARE_RANGES(result, (std::array{2, 4, 8, 10}));
      }
    }

    WHEN("elements partitioned")
    {
      const auto isEven = [](const auto& e) { return e % 2 == 0; };
      const auto &[resultTrue, resultFalse] = numbers.partition(isEven);
      THEN ("result contains even numbers")
      {
        COMPARE_RANGES(resultTrue, (std::array{2, 4, 6, 8, 10}));
        COMPARE_RANGES(resultFalse, (std::array{1, 3, 5, 7, 9}));
      }
    }

    WHEN("elements sliced within boundaries")
    {
      const auto result = numbers.slice(1, 3);
      THEN("contains the sliced elements")
      {
        COMPARE_RANGES(result, (std::array{2, 3}));
      }
    }

    WHEN("elements sliced within boundaries two times")
    {
      const auto result = numbers.slice(1, 5).slice(2, 3);
      THEN("contains the sliced elements")
      {
        COMPARE_RANGES(result, (std::array{4}));
      }
    }

    WHEN("elements sliced with larger ending boundary")
    {
      const auto result = numbers.slice(4, 15);
      THEN("contains the sliced elements")
      {
        COMPARE_RANGES(result, (std::array{5, 6, 7, 8, 9, 10}));
      }
    }

    WHEN("elements sliced with after starting boundary")
    {
      const auto result = numbers.slice(12, 14);
      THEN("contains the sliced elements")
      {
        COMPARE_RANGES(result, (std::array<int, 0>{}));
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
        COMPARE_RANGES(result, (std::array<int, 0>{}));
      }
    }

    WHEN("another range concatenated")
    {
      const std::vector<int> others{3, 4, 5};
      const auto result = numbers.concatenate(others);

      THEN("contains all elements")
      {
        COMPARE_RANGES(result, (std::array{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 3, 4, 5}));
      }
    }
    WHEN("another range concatenated multiple times")
    {
      const std::vector<int> others{3, 4, 5};
      const std::vector<int> more_others{6, 7, 8};
      const auto result = numbers.concatenate(others).concatenate(more_others);

      THEN("contains all elements")
      {
        COMPARE_RANGES(result, (std::array{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 3, 4, 5, 6, 7, 8}));
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
        COMPARE_RANGES(result, (std::array{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
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
        COMPARE_RANGES(as_strings, (std::array<std::string, 10>{"1:2", "2:4", "3:6", "4:8", "5:10", "6:12", "7:14", "8:16", "9:18", "10:20"}));
      }
    }

    //const auto to_string = [](auto&& e) { return std::to_string(e); }; // lifting
    WHEN("zipped")
    {
      const auto add_percent = [](const std::string& s) { return s + "%"; };
      const auto numbers_as_strings = numbers
        .map([](auto i) { return 2*i; })
        .map(ezy::to_string)
        .map(add_percent)
        .to<std::vector<std::string>>();

      const auto result = numbers.zip(numbers_as_strings);

      THEN("it is ok")
      {
        const auto as_strings = result
          .map([](const auto& p) { return std::to_string(std::get<0>(p)) + ":" + std::get<1>(p); });

        COMPARE_RANGES(as_strings,
            (std::array<std::string, 10>{"1:2%", "2:4%", "3:6%", "4:8%", "5:10%", "6:12%", "7:14%", "8:16%", "9:18%", "10:20%"}));
      }
    }

    WHEN("zipped multiple")
    {
      const auto zipped = numbers.zip(numbers.map([](int i) {return 10 + i;}), numbers.map([](int i){return 100 + i;}));
      //const auto zipped = numbers.zip(numbers.map([](int i) {return 10 + i;}), numbers);
      //const auto zipped = numbers.zip(numbers, numbers);
      const auto as_strings = zipped.map([](const auto& t) {
          const auto&[a,b,c] = t;
          return std::to_string(a) + ":" + std::to_string(b) + ":" + std::to_string(c);
        });

      COMPARE_RANGES(as_strings, (std::array<std::string, 10>{
            "1:11:101",
            "2:12:102",
            "3:13:103",
            "4:14:104",
            "5:15:105",
            "6:16:106",
            "7:17:107",
            "8:18:108",
            "9:19:109",
            "10:20:110"
            }))
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
          (std::array{1, 2, 3, 4, 5, 6}));

      COMPARE_RANGES((Raised{std::vector{1, 2, 3}, std::vector<int>{}, std::vector<int>{}, std::vector{4, 5, 6}}).flatten(),
          (std::array{1, 2, 3, 4, 5, 6}));

      COMPARE_RANGES(Raised{}.flatten(),
          (std::array<int, 0>{}));
    }

    WHEN("elements flatmapped")
    {
      using Raised = ezy::strong_type< std::vector<std::vector<int>>, struct RaisedTag, ezy::features::iterable>;
      const auto plus20 = [](int i) { return i + 20; };
      const auto result = Raised{std::vector{1, 2, 3}, std::vector{100, 101, 102}}.flat_map(plus20);
      THEN("contains flat list")
      {
        COMPARE_RANGES(result, (std::array{21, 22, 23, 120, 121, 122}));
      }
    }

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

    WHEN("take")
    {
      COMPARE_RANGES(numbers.take(1), (std::array{1}));
      COMPARE_RANGES(numbers.take(2), (std::array{1, 2}));
      COMPARE_RANGES(numbers.take(5), (std::array{1, 2, 3, 4, 5}));
      COMPARE_RANGES(numbers.take(10), (std::array{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
      COMPARE_RANGES(numbers.take(15), (std::array{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
    }

    WHEN("take multiple times")
    {
      COMPARE_RANGES(numbers.take(8).take(5).take(2), (std::array{1, 2}));
    }

    WHEN("take_while (<5)")
    {
      COMPARE_RANGES(numbers.take_while([](int i) {return i < 5;}), (std::array{1, 2, 3, 4}));
    }
    WHEN("take_while (>5)")
    {
      THEN("nothing returned")
      {
        COMPARE_RANGES(numbers.take_while([](int i) {return i > 5;}), (std::array<int, 0>{}));
      }
    }
    WHEN("take_while (<15)")
    {
      THEN("the whole range returned")
      {
        COMPARE_RANGES(numbers.take_while([](int i) {return i < 15;}), (std::array{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
      }
    }
    // take until?

    //
    // TODO fold
    // TODO span(unary): similar to partition, just splitting at first !unary(element)
    // TODO break(unary): span with inverted condition
    // TODO drop(n: index): removes the first n elements
    // TODO split_at(index)
    // TODO head/tail
    // TODO unzip
    // TODO grouping?
    //

    GIVEN("a string vector")
    {
      using MyStrings = ezy::strong_type<std::vector<std::string>, void, ezy::features::iterable>;
      MyStrings strings{"alma", "korte", "szilva"};
      WHEN("its joined with empty separator")
      {
        REQUIRE(strings.join("") == "almakorteszilva");
      }
      WHEN("its joined with separator")
      {
        REQUIRE(strings.join("/") == "alma/korte/szilva");
      }
    }

    GIVEN("a const string vector")
    {
      using MyStrings = ezy::strong_type<std::vector<std::string>, void, ezy::features::iterable>;
      const MyStrings strings{"alma", "korte", "szilva"};
      WHEN("its joined with empty separator")
      {
        REQUIRE(strings.join("") == "almakorteszilva");
      }
      WHEN("its joined with separator")
      {
        REQUIRE(strings.join("/") == "alma/korte/szilva");
      }
    }

    WHEN("mapped and joined")
    {
      REQUIRE(numbers.map(ezy::to_string).join(".") == "1.2.3.4.5.6.7.8.9.10");
    }

    WHEN("mapped then filtered")
    {
      auto transform = [](auto i) { return i + 10; };
      auto is_even = [](auto i) { return (i % 2) == 0; };

      const auto result = numbers
        .map(transform)
        .filter(is_even);

      COMPARE_RANGES(result, (std::array<int, 5>{12,14,16,18,20}));
    }

    WHEN("filtered then mapped")
    {
      auto transform = [](auto i) { return i + 10; };
      auto is_even = [](auto i) { return (i % 2) == 0; };

      const auto result = numbers
        .filter(is_even)
        .map(transform);

      COMPARE_RANGES(result, (std::array<int, 5>{12,14,16,18,20}));
    }
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

