#include <catch2/catch.hpp>

#include <ezy/string.h>

SCENARIO("to_string proxies to std::to_string")
{
  REQUIRE(ezy::to_string(123) == "123");
  REQUIRE(ezy::to_string(-200) == "-200");
  REQUIRE(ezy::to_string(1234455l) == "1234455");
  REQUIRE(ezy::to_string(1234455111222ll) == "1234455111222");
  REQUIRE(ezy::to_string(1234455111222u) == "1234455111222");
  REQUIRE(ezy::to_string(1234455111222333ul) == "1234455111222333");
  REQUIRE(ezy::to_string(1234455111222333444ull) == "1234455111222333444");
  REQUIRE(ezy::to_string(-123.25f) == "-123.250000");
  REQUIRE(ezy::to_string(-1234.25) == "-1234.250000");
  REQUIRE(ezy::to_string(-1234.25l) == "-1234.250000");
}

SCENARIO("to_string can construct a string from char*")
{
  static_assert(std::is_same<decltype(ezy::to_string("Hello")), std::string>::value);
  REQUIRE(ezy::to_string("Hello") == "Hello");

  const char* chars = "world!";
  const auto str = ezy::to_string(chars);
  static_assert(std::is_same<decltype(str), const std::string>::value);
  REQUIRE(str == "world!");

  static_assert(std::is_same<decltype(ezy::to_string(str.data())), std::string>::value);
  REQUIRE(ezy::to_string(str.data()) == "world!");
}

struct my_struct
{};

std::string to_string(const my_struct&)
{
  return "MyStruct{}";
}

SCENARIO("to_string calls to_string free function")
{
  REQUIRE(ezy::to_string(my_struct{}) == "MyStruct{}");
}
