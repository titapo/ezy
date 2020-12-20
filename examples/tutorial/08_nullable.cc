#include <ezy/strong_type.h>
#include <ezy/features/nullable.h>
#include <ezy/experimental/value_provider.h>


void default_nullable()
{
  using DefaultNullable = ezy::strong_type<int, struct DefaultNullableTag, ezy::features::experimental::default_nullable>;
  static_assert(DefaultNullable{15}.has_value());
  static_assert(!DefaultNullable{}.has_value());
  static_assert(!DefaultNullable{0}.has_value());

  static_assert(DefaultNullable{15}.value() == 15);
  static_assert(DefaultNullable{}.value() == 0);
}

void minus_1()
{
  using NullableAs = ezy::strong_type<int, struct NullableAsTag,
        ezy::features::experimental::nullable_as<ezy::experimental::value_provider<-1>>>;

  static_assert(NullableAs{15}.has_value());
  static_assert(NullableAs{}.has_value());
  static_assert(!NullableAs{-1}.has_value());

  static_assert(NullableAs{15}.value() == 15);
  static_assert(NullableAs{-1}.value() == -1);

  static_assert(NullableAs::make_null().has_value() == false);
  static_assert(NullableAs::make_null().value() == -1);
}

void custom_comparison()
{
  using Nullable = ezy::strong_type<int, struct Tag,
        ezy::features::experimental::nullable_as<
          ezy::experimental::value_provider<-1>,
          std::less_equal<>
        >
      >;

  static_assert(Nullable{10}.has_value());
  static_assert(Nullable{}.has_value());
  static_assert(!Nullable{-1}.has_value());
  static_assert(!Nullable{-10}.has_value());

  static_assert(Nullable::is_consistent());
}

#include <string>
#include <ezy/bits/empty_size.h> // TODO move out from bits
#include <cassert>
#include <iostream>

template <auto Fn>
struct fn_wrap
{
  template <typename...Args>
  constexpr bool operator()(Args&&...args) const
  {
    return ezy::invoke(Fn, args...);
  }
};

bool str_is_empty(const std::string& str)
{
  return str.empty();
}

void custom_condition()
{
  using NullableStr = ezy::strong_type<std::string, struct Tag,
        ezy::features::experimental::nullable_if<
          // does not compile:
          // decltype(&std::string::empty) does not compile -> it would crash
          //decltype(str_is_empty) // -> compound literal type: I think we cannot do better now

          // working alternatives:
          //fn_wrap<&std::string::empty> // -> fn_wrap is not in ezy yet
          fn_wrap<str_is_empty> // -> fn_wrap is not in ezy yet
          //ezy::empty_fn // works for this special case
          >
    >;

  assert(!NullableStr{}.has_value());
  assert(NullableStr{"hello world"}.has_value());
}


int main()
{
  default_nullable();
  minus_1();
  custom_comparison();
  custom_condition();
  return 0;
}
