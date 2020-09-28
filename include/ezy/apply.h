#ifndef EZY_APPLY_H_INCLUDED
#define EZY_APPLY_H_INCLUDED

#include "invoke.h"
#include <cstddef>
#include <tuple>

/**
 * ezy::apply is a polyfill for pre-c++17 clients. For newer version std::apply is recommended
 * */
namespace ezy
{
  namespace detail
  {
    template <typename Fn, typename Tuple, size_t... Is>
    constexpr decltype(auto) apply_impl(Fn&& fn, Tuple&& tuple, std::index_sequence<Is...>)
    {
      return ezy::invoke(std::forward<Fn>(fn), std::get<Is>(std::forward<Tuple>(tuple))...);
    }
  }

  template <typename Fn, typename Tuple>
  constexpr decltype(auto) apply(Fn&& fn, Tuple&& tuple)
  {
    return detail::apply_impl(
        std::forward<Fn>(fn),
        std::forward<Tuple>(tuple),
        std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>()
        );
  }
}
#endif
