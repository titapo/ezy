
#include <ezy/strong_type>
#include <ezy/features/common.h>
#include <ezy/features/printable.h>
#include <ezy/string.h>

// TODO think of: making ezy::optional and ezy::result iterable?

#include <ezy/result.h>

#include "common.h"

#include <catch2/catch.hpp>

static_assert(!std::is_copy_constructible_v<move_only>);
static_assert(!std::is_trivially_copy_constructible_v<move_only>);
static_assert(!std::is_copy_assignable_v<move_only>);
static_assert(!std::is_trivially_copy_assignable_v<move_only>);

static_assert(std::is_move_constructible_v<move_only>);
static_assert(std::is_trivially_move_constructible_v<move_only>);
static_assert(std::is_move_assignable_v<move_only>);
static_assert(std::is_trivially_move_assignable_v<move_only>);

SCENARIO("stong type preserves move_only's construction attributes")
{
  using Strengthten = ezy::strong_type<move_only, struct Tag>;

  static_assert(!std::is_copy_constructible_v<Strengthten>);
  static_assert(!std::is_trivially_copy_constructible_v<Strengthten>);
  static_assert(!std::is_copy_assignable_v<Strengthten>);
  static_assert(!std::is_trivially_copy_assignable_v<Strengthten>);

  static_assert(std::is_move_constructible_v<Strengthten>);
  static_assert(std::is_trivially_move_constructible_v<Strengthten>);
  static_assert(std::is_move_assignable_v<Strengthten>);
  static_assert(std::is_trivially_move_assignable_v<Strengthten>);
}


static_assert(!std::is_copy_constructible_v<non_transferable>);
static_assert(!std::is_trivially_copy_constructible_v<non_transferable>);
static_assert(!std::is_copy_assignable_v<non_transferable>);
static_assert(!std::is_trivially_copy_assignable_v<non_transferable>);


static_assert(!std::is_move_constructible_v<non_transferable>);
static_assert(!std::is_trivially_move_constructible_v<non_transferable>);
static_assert(!std::is_move_assignable_v<non_transferable>);
static_assert(!std::is_trivially_move_assignable_v<non_transferable>);

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
  auto twice_mutable = [](int& i) {return i*2;};
  auto twice_move = [](move_only&& m) {return move_only{m.i*2};};
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

  GIVEN("success")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    REQUIRE(R{10}.success() == 10);
    const R r{11};
    REQUIRE(r.success() == 11);
    R mutable_r{12};
    REQUIRE(mutable_r.success() == 12);
  }

  GIVEN("error")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    REQUIRE(R{"alma"}.error() == "alma");
    const R r{"banan"};
    REQUIRE(r.error() == "banan");
    R mutable_r{"korte"};
    REQUIRE(mutable_r.error() == "korte");
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

  GIVEN("map<Result>")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    using Opt = ezy::strong_type<std::optional<int>, void, ezy::features::result_interface<ezy::features::optional_adapter>>;
    const R r{11};
    REQUIRE(r.map<Opt>(twice).get().value() == 22);
  }

  GIVEN("map<Result> on mutable")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    using Opt = ezy::strong_type<std::optional<int>, void, ezy::features::result_interface<ezy::features::optional_adapter>>;
    R r{12};
    REQUIRE(r.map<Opt>(twice_mutable).get().value() == 24);
  }

  GIVEN("map<Result> on rvalue-ref")
  {
    using R = ezy::strong_type<std::variant<move_only, std::string>, void, ezy::features::result_like_continuation>;
    using Opt = ezy::strong_type<std::optional<move_only>, void, ezy::features::result_interface<ezy::features::optional_adapter>>;
    REQUIRE(R{move_only{9}}.map<Opt>(twice_move).get().value().i == 18);
  }

  GIVEN("map_error")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    const R r{"error"};
    auto res = r.map_error([](const std::string &s){ return s.size(); });
    static_assert(std::is_same_v<
        decltype(res),
        ezy::strong_type<std::variant<int, size_t>, void, ezy::features::result_like_continuation>
        >);
    REQUIRE(std::get<size_t>(res.get()) == 5);
  }

  GIVEN("map_error on mutable")
  {
    using R = ezy::strong_type<std::variant<bool, int>, void, ezy::features::result_like_continuation>;
    R r{12};
    auto res = r.map_error(twice_mutable);
    static_assert(std::is_same_v<
        decltype(res),
        R
        >);
    REQUIRE(std::get<int>(res.get()) == 24);
  }

  GIVEN("map_error on move-only rvalue")
  {
    using R = ezy::strong_type<std::variant<bool, move_only>, void, ezy::features::result_like_continuation>;
    REQUIRE(std::get<move_only>(R{move_only{7}}.map_error(twice_move).get()).i == 14);
  }

  GIVEN("map_error<Result> from optional")
  {
    constexpr auto null_to_str = [](std::nullopt_t) { return "none"; };
    using Opt = ezy::strong_type<std::optional<int>, void, ezy::features::result_interface<ezy::features::optional_adapter>>;
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    WHEN("called on {21}")
    {
      const Opt opt{21};
      auto res = opt.map_error<R>(null_to_str);
      REQUIRE(std::get<int>(res.get()) == 21);
    }

    WHEN("called on {nullopt}")
    {
      const Opt opt;
      auto res = opt.map_error<R>(null_to_str);
      REQUIRE(std::get<std::string>(res.get()) == "none");
    }
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
    auto change = [](int i) -> R2 {if (i % 3 == 0) return R2{i / 3.0}; else return R2{"oops"};};
    REQUIRE(std::get<double>(R{9}.and_then(change).get()) == 3.0);
    REQUIRE(std::get<double>(R{9}.and_then(change).and_then(change).get()) == 1.0);
  }

  GIVEN("and_then -- changing type")
  {
    using R = ezy::strong_type<std::variant<int, std::string>, void, ezy::features::result_like_continuation>;
    using R2 = ezy::rebind_strong_type_t<R, std::variant<double, std::string>>;
    auto change = [](int i) -> R2 {if (i % 3 == 0) return R2{i / 3.0}; else return R2{"oops"};};
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

  GIVEN("tee and map")
  {
    using R = ezy::result<int, std::string>;
    const R r{10};
    auto result = 0;
    const int value = r.tee([&result](int i) { result += i; }).map([](int i) {return i / 2;}).success();
    REQUIRE(result == 10);
    REQUIRE(value == 5);
  }

  GIVEN("tee on error")
  {
    using R = ezy::result<int, std::string>;
    const R r{"error!"};
    auto result = 0;
    r.tee([&result](int i) { result += i; });
    REQUIRE(result == 0);
  }

  GIVEN("tee on rvalue")
  {
    using R = ezy::result<int, std::string>;
    auto result = 0;
    R{11}.tee([&result](const int& i) { result += i; });
    REQUIRE(result == 11);
  }
}

SCENARIO("result like interface for std::optional")
{
  auto twice = [](int i) {return i*2;};

  using O = ezy::strong_type<std::optional<int>, void, ezy::features::result_interface<ezy::features::optional_adapter>>;
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
    using Om = ezy::strong_type<std::optional<move_only>, void, ezy::features::result_interface<ezy::features::optional_adapter>>;
    move_only result{4};
    REQUIRE(Om{move_only{10}}.map([&](move_only&& m) { result = std::move(m); return 2.0;}).get().value() == 2.0);
    REQUIRE(result.i == 10);
  }

  WHEN("map_or called on it")
  THEN("string not need to be passed on parameter side") // not created internally
  {
    using O = ezy::strong_type<std::optional<int>, void, ezy::features::result_interface<ezy::features::optional_adapter>>;
    auto result = O{std::nullopt}.map_or(ezy::to_string, "foo");
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
    using Om = ezy::strong_type<std::optional<move_only>, void, ezy::features::result_interface<ezy::features::optional_adapter>>;
    move_only result{4};
    REQUIRE(Om{move_only{10}}.and_then([&](move_only&& m) { result = std::move(m); return Om{2};}).get().value().i == 2);
    REQUIRE(result.i == 10);
  }

  GIVEN("tee and map")
  {
    using Opt = ezy::optional<int>;
    const Opt opt{10};
    int result{0};
    const int value = opt.tee([&result](int i) { result += i; }).map([](int i) {return i / 2;}).value();
    REQUIRE(result == 10);
    REQUIRE(value == 5);
  }

  GIVEN("tee on nullopt")
  {
    using Opt = ezy::optional<int>;
    const Opt r{std::nullopt};
    auto result = 0;
    r.tee([&result](int i) { result += i; });
    REQUIRE(result == 0);
  }

}

/**
 * strong type general
 */
template <typename T, typename From>
struct is_explicit_constructible : std::conjunction<
                                   std::is_constructible<T, From>,
                                   std::negation<std::is_convertible<From, T>>
                                   > {};

template <typename T, typename From>
constexpr bool is_explicit_constructible_v = is_explicit_constructible<T, From>::value;

template <typename T, typename From>
struct is_implicit_constructible : std::conjunction<
                                   std::is_constructible<T, From>,
                                   std::is_convertible<From, T>
                                   > {};

template <typename T, typename From>
constexpr bool is_implicit_constructible_v = is_implicit_constructible<T, From>::value;

static_assert( is_explicit_constructible_v<ezy::strong_type<int, struct Tag>, int>);
static_assert( is_explicit_constructible_v<ezy::strong_type<int, struct Tag>, double>); // TODO control to handle if it's allowed or not
static_assert(!is_explicit_constructible_v<ezy::strong_type<int, void>, int>);
static_assert(!is_explicit_constructible_v<ezy::strong_type<int, void>, double>); // TODO control to handle if it's allowed or not

static_assert(!is_implicit_constructible_v<ezy::strong_type<int, struct Tag>, int>);
static_assert(!is_implicit_constructible_v<ezy::strong_type<int, struct Tag>, double>);
static_assert( is_implicit_constructible_v<ezy::strong_type<int, void>, int>);
static_assert( is_implicit_constructible_v<ezy::strong_type<int, void>, double>);

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

SCENARIO("strong type for int")
{
  using ST = ezy::strong_type<int, void>;

  WHEN("constructed without initializer")
  {
    ST i;
    THEN("it holds default initialized value")
    {
      REQUIRE(i.get() == 0);
    }
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

SCENARIO("strong type for a move only type")
{
  using ST = ezy::strong_type<move_only, struct DummyTag>;

  WHEN("constructed from underlying type")
  {
    const ST st{move_only{1}};
    THEN("it stores the proper value")
    {
      REQUIRE(st.get().i == 1);
    }
  }

  WHEN("constructed as its underlying value")
  {
    const ST st{2};
    THEN("it stores the proper value")
    {
      REQUIRE(st.get().i == 2);
    }
  }
}

SCENARIO("strong_type_reference construction")
{
  //using STR = ezy::strong_type_reference<int, struct DummyTag>;

  WHEN("construct reference from prvalue should not compile")
  {
    // STR s{15}; // OK: does not compile
  }

  WHEN("construct reference from rvalue should not compile")
  {
    // int i{16};
    // STR s{std::move(i)}; // OK: does not compile
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

SCENARIO("make_strong")
{
  int mutable_int = 1;
  const int const_int = 2;

  WHEN("make_strong")
  {
    static_assert(std::is_same_v<
        decltype(ezy::make_strong<struct Tag>(mutable_int)),
        ezy::strong_type<int, struct Tag>
      >);

    static_assert(std::is_same_v<
        decltype(ezy::make_strong<struct Tag>(const_int)),
        ezy::strong_type<int, struct Tag>
      >);

    static_assert(std::is_same_v<
        decltype(ezy::make_strong<struct Tag>(1)),
        ezy::strong_type<int, struct Tag>
      >);
  }

  WHEN("make_strong_const")
  {
    static_assert(std::is_same_v<
        decltype(ezy::make_strong_const<struct Tag>(mutable_int)),
        ezy::strong_type<const int, struct Tag>
      >);

    static_assert(std::is_same_v<
        decltype(ezy::make_strong_const<struct Tag>(const_int)),
        ezy::strong_type<const int, struct Tag>
      >);

    static_assert(std::is_same_v<
        decltype(ezy::make_strong_const<struct Tag>(1)),
        ezy::strong_type<const int, struct Tag>
      >);
  }

  WHEN("make_strong_reference")
  {
    static_assert(std::is_same_v<
        decltype(ezy::make_strong_reference<struct Tag>(mutable_int)),
        ezy::strong_type_reference<int, struct Tag>
      >);

    static_assert(std::is_same_v<
        decltype(ezy::make_strong_reference<struct Tag>(const_int)),
        ezy::strong_type_reference<const int, struct Tag>
      >);

    // ezy::make_strong_reference<struct Tag>(1) // OK does not compile
  }

  WHEN("make_strong_reference_const")
  {
    static_assert(std::is_same_v<
        decltype(ezy::make_strong_reference_const<struct Tag>(mutable_int)),
        ezy::strong_type_reference<const int, struct Tag>
      >);

    static_assert(std::is_same_v<
        decltype(ezy::make_strong_reference_const<struct Tag>(const_int)),
        ezy::strong_type_reference<const int, struct Tag>
      >);

    /**
     * OK: these are not compiling anymore:
    auto cref = ezy::make_strong_reference_const<struct Tag>(12345);
    int movable_int = 15;
    auto moved = ezy::make_strong_reference_const<struct Tag>(std::move(movable_int));
    */
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

SCENARIO("extended_type_reference")
{
  int i{3};
  WHEN("extended_type_reference created")
  {
    ezy::extended_type_reference<int> ref{i};
    i++;
    REQUIRE(ref.get() == 4);
  }

  WHEN("make_extended")
  {
    auto ext = ezy::make_extended<ezy::features::addable>(4);
    auto added = ext + ext;
    static_assert(std::is_same_v<decltype(added), ezy::extended_type<int, ezy::features::addable>>);
    REQUIRE(added.get() == 8);
  }

  WHEN("make_extended_const")
  {
    auto ext = ezy::make_extended_const<ezy::features::addable>(6);
    auto added = ext + ext;
    static_assert(std::is_same_v<decltype(added), ezy::extended_type<const int, ezy::features::addable>>);
    REQUIRE(added.get() == 12);
  }

  WHEN("make_extended_reference")
  {
    auto ext = ezy::make_extended_reference(i);
    ext.get() += 7;
    REQUIRE(i == 10);
  }

  WHEN("make_extended_reference_const")
  {
    auto ext = ezy::make_extended_reference_const(i);
    i += 4;
    REQUIRE(ext.get() == 7);
  }

}

SCENARIO("subscript operator")
{
  using ST = ezy::strong_type<std::vector<int>, struct DummyTag, ezy::features::operator_subscript>;

  GIVEN("a mutable vector with 4 elements")
  {
    ST st{1,2,3,4};
    THEN("accessed")
    {
      REQUIRE(st[2] == 3);
      ++st[3];
      REQUIRE(st[3] == 5);
    }
  }

  GIVEN("a vector with 4 elements")
  {
    const ST st{2,3,4,5};
    THEN("accessed")
    {
      REQUIRE(st[2] == 4);
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
      REQUIRE(other.get() == 123);
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

template <typename >struct pt;
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

  WHEN("closed multipliable")
  {
    using Mult = ezy::strong_type<int, struct Tag, ezy::features::closed_multipliable>;
    THEN("multiplication works and returns the same type (closed multiplication)")
    {
      Mult a(5);
      Mult b(3);
      static_assert(std::is_same_v<Mult, decltype(a * b)>);
      REQUIRE((a * b).get() == 15);
    }

    THEN("multiplicative update works and returns the same type (closed multiplication)")
    {
      Mult a(5);
      a *= Mult(3);
      REQUIRE(a.get() == 15);
    }
  }

  WHEN("multipliable")
  {
    using Mult = ezy::strong_type<int, struct Tag, ezy::features::multipliable>;
    THEN("multiplication works and returns the same type (closed multiplication)")
    {
      Mult a(5);
      int b{3};
      static_assert(std::is_same_v<Mult, decltype(a * b)>);
      REQUIRE((a * b).get() == 15);
    }

    THEN("multiplicative update works and returns the same type (closed multiplication)")
    {
      Mult a(5);
      a *= 3;
      REQUIRE(a.get() == 15);
    }

    THEN("multiplication works and returns the same type (closed multiplication) (swapped order)")
    {
      Mult a(5);
      int b{3};
      static_assert(std::is_same_v<Mult, decltype(b * a)>);
      REQUIRE((b * a).get() == 15);
    }

    THEN("multiply-assign works")
    {
      Mult a{5};
      a *= 4;
      REQUIRE(a.get() == 20);
    }
  }

  WHEN("closed_divisible")
  {
    using Div = ezy::strong_type<int, struct Tag, ezy::features::closed_divisible>;
    THEN("division returns the same type")
    {
      Div a{8};
      Div b{2};
      static_assert(std::is_same_v<Div, decltype(a/b)>);
      REQUIRE((a / b).get() == 4);
      a /= b;
      REQUIRE(a.get() == 4);
      REQUIRE(b.get() == 2);
    }
  }

  WHEN("division_by_results")
  {
    using Div = ezy::strong_type<int, struct Tag, ezy::features::division_by_results<float, double>>;
    Div a{10};
    static_assert(std::is_same_v<decltype(a/5.1), double>);
    static_assert(std::is_same_v<decltype(a/5), double>);
    // implicit cast of divisor is allowed now. Should we have a stricter version?
    REQUIRE((a / 4) == 2.5);
  }

  WHEN("divisible_by_scalar")
  {
    using Div = ezy::strong_type<int, struct Tag, ezy::features::divisible_by_scalar>;
    Div a{8};
    static_assert(std::is_same_v<decltype(a / 5), Div>);
    REQUIRE((a / 5).get() == 1);
    a /= 2; // this should work
    REQUIRE(a.get() == 4);
  }

  WHEN("division_results_scalar")
  {
    using Div = ezy::strong_type<int, struct Tag, ezy::features::division_results_scalar>;
    Div a{8};
    Div b{6};
    static_assert(std::is_same_v<decltype(a / b), int>);
    REQUIRE((a / b) == 1);
  }

  WHEN("divisible")
  {
    using Div = ezy::strong_type<int, struct Tag, ezy::features::divisible>;
    Div a{8};
    Div b{6};
    static_assert(std::is_same_v<decltype(a / b), int>);
    static_assert(std::is_same_v<decltype(a / 2), Div>);
    REQUIRE((a / b) == 1);
    REQUIRE((a / 2).get() == 4);
    a /= 4;
    REQUIRE(a.get() == 2);
  }
}

SCENARIO("negatable")
{
  using Neg = ezy::strong_type<int, struct Tag, ezy::features::negatable>;
  const Neg n{10};
  const auto result = -n;
  static_assert(std::is_same_v<decltype(result), const Neg>);
  REQUIRE(result.get() == -10);
}

SCENARIO("negatable should not work with char")
{
  // compilation fails with narrowing conversion
  //using Neg = ezy::strong_type<char, void, ezy::features::negatable>;
}

SCENARIO("strong type comparisons")
{
  WHEN("equal_comparable")
  {
    using EqCmp = ezy::strong_type<int, struct Tag, ezy::features::equal_comparable, ezy::features::printable>;
    THEN("eq works")
    {
      EqCmp a(5);
      EqCmp b(6);
      EqCmp c(5);

      REQUIRE(a == a);
      REQUIRE(a != b);
      REQUIRE(a == c);
    }

  }

  GIVEN("a type without != operator")
  {
    struct EqOpOnly
    {
      constexpr bool operator==(const EqOpOnly&) const noexcept
      { return false; }
    };

    // TODO check noexcept propagation
    using EqCmpOnly = ezy::strong_type<EqOpOnly, struct Tag, ezy::features::equal_comparable>;
    WHEN("not equal called")
    THEN("it's generated automagically")
    {
      REQUIRE((EqCmpOnly{} != EqCmpOnly{}));
    }

  }

  GIVEN("a type with explicitt!= operator")
  {
    struct SpecNotEq
    {
      constexpr bool operator==(const SpecNotEq&) const noexcept
      { return false; }

      constexpr bool operator!=(const SpecNotEq&) const noexcept
      { return false; }
    };

    using EqSpecNotEq = ezy::strong_type<SpecNotEq, struct Tag, ezy::features::equal_comparable>;
    WHEN("not equal called")
    THEN("invoked the original one")
    {
      REQUIRE(!(EqSpecNotEq{} == EqSpecNotEq{}));
      REQUIRE(!(EqSpecNotEq{} != EqSpecNotEq{}));
    }
  }

  GIVEN("greater")
  {
    using G = ezy::strong_type<int, struct Tag, ezy::features::greater, ezy::features::printable>;

    REQUIRE(G{5} > G{4});
    REQUIRE(!(G{5} > G{5}));

    using GE = ezy::strong_type<int, struct Tag, ezy::features::greater_equal, ezy::features::printable>;
    REQUIRE(GE{5} >= GE{4});
    REQUIRE(GE{5} >= GE{5});

    using L = ezy::strong_type<int, struct Tag, ezy::features::less, ezy::features::printable>;

    REQUIRE(L{4} < L{5});
    REQUIRE(!(L{5} < L{5}));

    using LE = ezy::strong_type<int, struct Tag, ezy::features::less_equal, ezy::features::printable>;
    REQUIRE(LE{4} <= LE{5});
    REQUIRE(LE{5} <= LE{5});
  }
}

#include <ezy/compose.h>
#include <ezy/experimental/function>

const auto str_plus_int = [](const std::string& s, const int i)
{ return s + std::to_string(i); };

SCENARIO("curry")
{
  using namespace ezy::experimental;
  using namespace std::string_literals;

  GIVEN("a function object")
  {
    WHEN("curried")
    {
      const auto addInt = curried{std::plus<int>{}};
      const auto addFive = addInt(5);
      REQUIRE(addFive(10) == 15);
    }
  }

  GIVEN("a generic function object")
  {
    const auto add = curried{std::plus<>{}};
    REQUIRE(add(3)(5) == 8);
    REQUIRE(add("a"s)("10") == "a10");
  }

  GIVEN("a unary member function")
  {
    const auto length = curried{&std::string::size};
    REQUIRE(length(""s) == 0);
    REQUIRE(length("hello"s) == 5);
  }

  // TODO this is a different concept: rename to lazy value or similar
  GIVEN("lazy value")
  {
    const auto five = curried{5};
    REQUIRE(five() == 5);
  }

  // curry(nullary) -> not accepted

  GIVEN("function with 3 parameters")
  {
    auto sum3 = curried_with_args{[](int i, int j, int k) { return i + j + k; }};
    REQUIRE(sum3(10)(5)(-20) == -5);

  }

  GIVEN("a lambda with different types")
  {
    REQUIRE(str_plus_int("a", 2) == "a2");

    WHEN("curried")
    {
      const auto curried = curry(str_plus_int);
      const auto prefixed = curried("number: ");
      REQUIRE(prefixed(123) == "number: 123");
    }
  }

  GIVEN("currying works in constexpr context")
  {
    constexpr auto add = curried{std::plus<>{}};
    constexpr auto add5 = add(5);
    static_assert(add5(10) == 15);
  }
}

SCENARIO("flip")
{
  using ezy::experimental::flip;
  using namespace std::string_literals;
  const auto append = flip(std::plus<>{});
  REQUIRE(append("foo"s, "bar"s) == "barfoo");
}

int add_5(int in)
{
  return in + 5;
}

SCENARIO("compose regular function")
{

  auto composed = ezy::compose(add_5, add_5);
  REQUIRE(composed(4) == 14);
}

SCENARIO("compose regular function and invoke in one expression")
{
  REQUIRE(ezy::compose(add_5, add_5)(2) == 12);
}

SCENARIO("compose")
{
  using namespace ezy::experimental;
  auto addHundred = curry_as<int, int>(std::plus<int>{})(100);

  WHEN("composed with same type")
  {
    auto addTwoHundred = ezy::compose(addHundred, addHundred);
    REQUIRE(addTwoHundred(23) == 223);
  }

  WHEN("composed with different type")
  {
    // TODO const lambdas are not accepted!
    auto formatNumber = curry_as<std::string, int>(str_plus_int)("result is: ");
    auto calculate = ezy::compose(addHundred, formatNumber);
    REQUIRE(calculate(23) == "result is: 123");
  }

  WHEN("access member")
  {
    struct S{ int i; };

    auto calculate = ezy::compose(&S::i, addHundred);

    S s{3};
    REQUIRE(calculate(s) == 103);
  }

  WHEN("invoked in constexpr context")
  {
    // 'static' is required here.
    static constexpr auto plus_4 = [](auto i) {return i + 4;};
    static constexpr auto plus_10 = [](auto i) {return i + 10;};

    constexpr auto plus_14 = ezy::compose(plus_4, plus_10);
    static_assert(plus_14(6) == 20);
  }

  WHEN("composing more")
  {
    auto double_length = ezy::compose(ezy::to_string, &std::string::size, [](auto s) {return s * 2;});

    REQUIRE(double_length(4) == 2);
    REQUIRE(double_length(12345) == 10);
    REQUIRE(ezy::compose(double_length, double_length)(12345) == 4);
  }
  // moving etc
}

SCENARIO("pickers")
{
  GIVEN("a tuple")
  {
    std::tuple<int, double, std::string, int> t{10, 12.5, "hello", 8};
    WHEN("first picked")
    {
      REQUIRE(ezy::pick_nth<0>(t) == 10);
      REQUIRE(ezy::pick_first(t) == 10);
      //REQUIRE(ezy::pick_type<int>(t) == 10); // ambiguous
    }

    WHEN("second picked")
    {
      REQUIRE(ezy::pick_nth<1>(t) == 12.5);
      REQUIRE(ezy::pick_second(t) == 12.5);
      REQUIRE(ezy::pick_type<double>(t) == 12.5);
    }

    WHEN("third picked")
    {
      REQUIRE(ezy::pick_nth<2>(t) == "hello");
      REQUIRE(ezy::pick_type<std::string>(t) == "hello");
    }

    WHEN("fourth picked")
    {
      REQUIRE(ezy::pick_nth<3>(t) == 8);
    }

    WHEN("pick from a temporary")
    {
      auto picked = ezy::pick_second(std::tuple{2, 4, 6});
      REQUIRE(picked == 4);
      static_assert(std::is_same_v<decltype(picked), int>);
    }

    WHEN("pick a reference")
    {
      auto& picked = ezy::pick_second(t);
      REQUIRE(picked == 12.5);
      std::get<1>(t) -= 10.0;
      REQUIRE(picked == 2.5);
    }

    WHEN("pick a mutable reference by index")
    {
      auto& picked = ezy::pick_second(t);
      REQUIRE(picked == 12.5);
      picked += 10;
      REQUIRE(std::get<1>(t) == 22.5);
      REQUIRE(picked == 22.5);
    }

    WHEN("pick a mutable reference by type")
    {
      auto& picked = ezy::pick_type<double>(t);
      REQUIRE(picked == 12.5);
      picked += 10;
      REQUIRE(std::get<double>(t) == 22.5);
      REQUIRE(picked == 22.5);
    }
  }

}

#include <ezy/experimental/tuple_algorithm.h>

// TODO those things are implemented as strong type features
SCENARIO("static_for_each")
{
  std::tuple<int, char, double> t{1, 'b', 3.4};
  std::string result;
  ezy::experimental::static_for_each(t,
      ezy::overloaded{
        [&result] (int)    { result += "int ";},
        [&result] (char)   { result += "char ";},
        [&result] (double) { result += "double ";},
        [&result] (auto)   { result += "else ";},
      });
  REQUIRE(result == "int char double ");
}

SCENARIO("tuple_map")
{
  const std::tuple<int, char, double> t{1, 'b', 3.4};
  const auto result = ezy::experimental::tuple_map(t, [](auto i) -> decltype(i) {return i + 1;});
  REQUIRE(result == std::tuple{2, 'c', 4.4});
}

SCENARIO("tuple_fold")
{
  const std::tuple<int, double, int> t{1, 3.4, 2};
  const auto result = ezy::experimental::tuple_fold(t, 0, [](auto i, auto j) -> int {return i + j;});
  REQUIRE(result == 6);
}

SCENARIO("tuple_for_each_enumerate")
{
  using namespace std::string_literals;
  const std::tuple<int, unsigned, int> t{1, 20, 2};
  std::string result;
  ezy::experimental::tuple_for_each_enumerate(t,
      [&result](size_t i, auto&& e) { result += " "s + std::to_string(i) + ":" + std::to_string(e); });
  REQUIRE(result == " 0:1 1:20 2:2");
}

SCENARIO("tuple_for_each_enumerate as constexpr")
{
  using namespace std::string_literals;
  const std::tuple<int, unsigned, int> t{1, 20, 2};
  std::string result;
  static constexpr auto characters = std::tuple{'a', 'b', 'c', 'd'};
  ezy::experimental::tuple_for_each_enumerate(t,
      [&result](auto i, auto&& e) {
      result += " "s + std::to_string(i) + ":" + std::to_string(e) + "-" + std::get<i>(characters);
      });
  REQUIRE(result == " 0:1-a 1:20-b 2:2-c");
}

SCENARIO("tuple_zip_for_each")
{
  using namespace std::string_literals;
  const std::tuple<int, unsigned, int> t1{1, 20, 2};
  const std::tuple<char, char, char> t2{'a', 'b', 'c'};
  std::string result;
  ezy::experimental::tuple_zip_for_each(t1, t2,
      [&result](auto e1, auto e2) {
      result += " "s + std::to_string(e1) + "-" + e2;
      });
  REQUIRE(result == " 1-a 20-b 2-c");
}

SCENARIO("static_for")
{
  using namespace std::string_literals;

  const std::tuple<int, unsigned, int> t{1, 4u, 2};
  std::string result;
  ezy::experimental::static_for<3>([&](auto i)
      {
        result += " "s + std::to_string(i) + ":" + std::to_string(std::get<i>(t));
      });

   REQUIRE(result == " 0:1 1:4 2:2");
}

SCENARIO("handy size_of")
{
  using namespace std::string_literals;

  const std::tuple t{1, 4u, 10, true};
  static_assert(ezy::experimental::size_of(t) == 4);

  std::string result;
  ezy::experimental::static_for<ezy::experimental::size_of(t)>([&](auto i)
      {
        result += " "s + std::to_string(i) + ":" + std::to_string(std::get<i>(t));
      });

   REQUIRE(result == " 0:1 1:4 2:10 3:1");
}

