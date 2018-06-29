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
   
    //
    // TODO fold
    // TODO span(unary): similar to partition, just splitting at first !unary(element)
    // TODO break(unary): span with inverted condition
    // TODO drop(n: index): drop (and return) the first n elements <-- what is the difference between this and take
    //                     (mutability?)
    // TODO split_at(index)
    // TODO flatten
    WHEN("flattened")
    {
      // FIXME it seems that MyNumbers has no const_iterator
      using Raised = strong_type< std::vector<std::vector<int>>, struct RaisedTag, iterable>;
      const Raised raised{std::vector{1, 2, 3}, std::vector{4, 5, 6}};
      COMPARE_RANGES(raised.flatten(), (std::initializer_list{1, 2, 3, 4, 5, 6}));
      // COMPARE_RANGES(Raised{}.flatten(), (std::initializer_list<int>{})); FIXME segfault
    }
    // TODO head/tail
    // TODO take(n)?, take_while(predicate)
    // TODO unzip
    // TODO grouping?
    // TODO collect?
    // TODO empty
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

  // FIXME how?
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
    // FIXME make it work
    /*
    ST st{4, "str"};
    const ST other(std::move(st));
    THEN("its values are also moved")
    {
      REQUIRE(other.get().i == 4);
      REQUIRE(other.get().s == "str");
    }
    */
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
    // TODO check it
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
}
