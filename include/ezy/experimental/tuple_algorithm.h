#ifndef EZY_EXPERIMENTAL_TUPLE_ALGORITHM_H_INCLUDED
#define EZY_EXPERIMENTAL_TUPLE_ALGORITHM_H_INCLUDED

#include <utility>
#include <tuple>

namespace ezy::experimental
{
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

  template <typename Tuple, typename Fn>
  [[nodiscard]] decltype(auto) tuple_map(Tuple&& t, Fn&& fn)
  {
    return detail::tuple_map_impl(
        std::forward<Fn>(fn),
        std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{}
      );
  }
}

#endif
