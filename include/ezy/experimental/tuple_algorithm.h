#ifndef EZY_EXPERIMENTAL_TUPLE_ALGORITHM_H_INCLUDED
#define EZY_EXPERIMENTAL_TUPLE_ALGORITHM_H_INCLUDED

#include <utility>
#include <tuple>

namespace ezy::experimental
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
  void tuple_for_each(Tuple&& t, Fn&& fn)
  {
    std::apply([&fn](auto&&... e) { (fn(std::forward<decltype(e)>(e)), ...);}, std::forward<Tuple>(t));
  }

  namespace detail
  {
    template <typename Fn, typename Tuple, size_t... Is>
    [[nodiscard]] decltype(auto) tuple_map_impl(Fn&& fn, Tuple&& t, std::index_sequence<Is...>)
    {
      return std::tuple(std::invoke(fn, std::get<Is>(t))...);
    }
  }

  namespace detail
  {
    template <typename Tuple, typename Fn, size_t... Is>
    auto tuple_for_each_enumerate_helper(Tuple&& t, Fn&& fn, std::index_sequence<Is...>)
    {
      (
        std::invoke(
          std::forward<Fn>(fn),
          std::integral_constant<size_t, Is>{},
          std::get<Is>(std::forward<Tuple>(t))
        ), ...
      );
    }
  }

  template <typename Tuple, typename Fn>
  auto tuple_for_each_enumerate(Tuple&& t, Fn &&fn)
  {
    detail::tuple_for_each_enumerate_helper(
        std::forward<Tuple>(t),
        std::forward<Fn>(fn),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>()
    );
  }

  /**
   * mapping for tuple-like objects
   *
   * result is always an std::tuple (this may be relaxed)
   */
  template <typename Tuple, typename Fn>
  [[nodiscard]] decltype(auto) tuple_map(Tuple&& t, Fn&& fn)
  {
    return detail::tuple_map_impl(
        std::forward<Fn>(fn),
        std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{}
      );
  }

  /**
   * folding for tuple-like objects
   */
  template <typename Tuple, typename T, typename Op>
  [[nodiscard]] T tuple_fold(Tuple&& t, T init, Op&& op)
  {
    tuple_for_each(std::forward<Tuple>(t), [&init, &op](auto&& e){ init = std::invoke(op, init, e); });
    return init;
  }
}

#endif
