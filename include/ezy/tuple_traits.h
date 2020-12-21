#ifndef EZY_TUPLE_TRAITS_HH_INCLUDED
#define EZY_TUPLE_TRAITS_HH_INCLUDED

#include <tuple>
#include "typelist_traits.h"

namespace ezy { namespace tuple_traits
{
  template <typename>
  struct is_tuple: std::false_type {};

  template <typename... T>
  struct is_tuple<std::tuple<T...>>: std::true_type {};

  template <typename T>
  constexpr bool is_tuple_v = is_tuple<T>::value;

  /**
   * tuplify
   *
   *   tuplify makes a tuple
   *    tuple -> tuple (no change)
   *    void -> tuple<> (empty)
   *    other -> tuple<other>
   *
   */
  template <typename T>
  struct tuplify
  {
    using type = std::tuple<T>;
  };

  template <typename... Ts>
  struct tuplify<std::tuple<Ts...>>
  {
    using type = std::tuple<Ts...>;
  };

  //theoretically it does not correct unit_t should result std::tuple
  template <>
  struct tuplify<void>
  {
    using type = std::tuple<>;
  };

  template <typename T>
  using tuplify_t = typename tuplify<T>::type;
}}

#endif
