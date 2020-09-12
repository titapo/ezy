#ifndef EZY_EXPERIMENTAL_TUPLE_ALGORITHM_H_INCLUDED
#define EZY_EXPERIMENTAL_TUPLE_ALGORITHM_H_INCLUDED

#include "../type_traits.h"
#include "../invoke.h"
#include "../apply.h"

#include <utility>
#include <tuple>
#include <cstddef>

namespace ezy
{
namespace experimental
{
  /**
   * for_each for tuple-like objects
   *
   * pseudo code:
   *
   * for...(auto&& element : t)
   * {
   *    fn(element);
   * }
   */
  template <typename Tuple, typename Fn>
  constexpr void static_for_each(Tuple&& t, Fn&& fn)
  {
    std::apply([&fn](auto&&... e) { (fn(std::forward<decltype(e)>(e)), ...);}, std::forward<Tuple>(t));
  }

  namespace detail
  {
    template <typename Fn, size_t... Is>
    constexpr void static_for_helper(Fn&& fn, std::index_sequence<Is...>)
    {
      (
        std::forward<Fn>(fn)(std::integral_constant<size_t, Is>{}), ...
      );
    }
  }

  template <size_t Size, typename Fn>
  constexpr void static_for(Fn&& fn)
  {
    detail::static_for_helper(
        std::forward<Fn>(fn),
        std::make_index_sequence<Size>()
        );
  }

  namespace detail
  {
    template <typename Fn, typename Tuple, size_t... Is>
    [[nodiscard]] constexpr decltype(auto) tuple_map_impl(Fn&& fn, Tuple&& t, std::index_sequence<Is...>)
    {
      return std::tuple(ezy::invoke(fn, std::get<Is>(t))...);
    }
  }

  namespace detail
  {
    template <typename Tuple, typename Fn, size_t... Is>
    constexpr auto tuple_for_each_enumerate_helper(Tuple&& t, Fn&& fn, std::index_sequence<Is...>)
    {
      using std::get;
      (
        ezy::invoke(
          std::forward<Fn>(fn),
          std::integral_constant<size_t, Is>{},
          get<Is>(std::forward<Tuple>(t))
        ), ...
      );
    }
  }

  template <typename Tuple, typename Fn>
  constexpr auto tuple_for_each_enumerate(Tuple&& t, Fn &&fn)
  {
    detail::tuple_for_each_enumerate_helper(
        std::forward<Tuple>(t),
        std::forward<Fn>(fn),
        std::make_index_sequence<std::tuple_size<ezy::remove_cvref_t<Tuple>>::value>()
    );
  }

  /**
   * mapping for tuple-like objects
   *
   * result is always an std::tuple (this may be relaxed)
   */
  template <typename Tuple, typename Fn>
  [[nodiscard]] constexpr decltype(auto) tuple_map(Tuple&& t, Fn&& fn)
  {
    return detail::tuple_map_impl(
        std::forward<Fn>(fn),
        std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size<ezy::remove_cvref_t<Tuple>>::value>{}
      );
  }

  /**
   * folding for tuple-like objects
   */
  template <typename Tuple, typename T, typename Op>
  [[nodiscard]] constexpr T tuple_fold(Tuple&& t, T init, Op&& op)
  {
    static_for_each(std::forward<Tuple>(t), [&init, &op](auto&& e){ init = ezy::invoke(op, init, e); });
    return init;
  }

  namespace detail
  {
    template <typename Tuple1, typename Tuple2, typename Fn, size_t... Is>
    constexpr auto tuple_zip_for_each_helper(Tuple1&& t1, Tuple2&& t2, Fn&& fn, std::index_sequence<Is...>)
    {
      (
        ezy::invoke(
          std::forward<Fn>(fn),
          std::get<Is>(std::forward<Tuple1>(t1)),
          std::get<Is>(std::forward<Tuple2>(t2))
        ), ...
      );
    }
  }

  template <typename Tuple1, typename Tuple2, typename Fn>
  constexpr void tuple_zip_for_each(Tuple1&& t1, Tuple2&& t2, Fn&& fn)
  {
    static_assert(
        std::tuple_size<ezy::remove_cvref_t<Tuple1>>::value == std::tuple_size<ezy::remove_cvref_t<Tuple1>>::value,
        "mismatching size");

    detail::tuple_zip_for_each_helper(
        std::forward<Tuple1>(t1),
        std::forward<Tuple2>(t2),
        std::forward<Fn>(fn),
        std::make_index_sequence<std::tuple_size<ezy::remove_cvref_t<Tuple1>>::value>()
        );
  }

  template <typename T>
  constexpr size_t size_of(T&&) noexcept
  {
    return std::tuple_size<std::remove_cv_t<std::remove_reference_t<T>>>::value;
  }
}
}

#endif
