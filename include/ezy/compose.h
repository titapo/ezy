#ifndef EZY_COMPOSE_H_INCLUDED
#define EZY_COMPOSE_H_INCLUDED

#include "invoke.h"

namespace ezy
{
  /**
   * compose
   */
  template <typename... Fs>
  struct composed
  {
    static_assert(sizeof...(Fs) > 0, "Composing nothing is not supported.");

    using FsTuple = std::tuple<Fs...>;

    composed(const composed&) = default;
    composed(composed&&) = default;

    // SFINAE out copy and move ctors
    template <typename... Functions,
             std::enable_if_t<(std::is_constructible_v<FsTuple, Functions...>), bool> = true>
    constexpr explicit composed(Functions&&... fns)
      : fs(std::forward<Functions>(fns)...)
    {}

    FsTuple fs;

    template <typename...Fns, typename Fn, typename T>
    constexpr static decltype(auto) call_helper_pack(T&& t, Fn&& fn, Fns&&... fns)
    {
      if constexpr (sizeof...(Fns) == 0)
      {
        return ezy::invoke(std::forward<Fn>(fn), std::forward<T>(t));
      }
      else
      {
        return call_helper_pack(ezy::invoke(std::forward<Fn>(fn), std::forward<T>(t)), std::forward<Fns>(fns)...);
      }
    }

    template <typename T, typename Tuple, std::size_t...Is>
    constexpr static decltype(auto) call_helper_tuple(T&& t, Tuple&& tup, std::index_sequence<Is...>)
    {
      return call_helper_pack(std::forward<T>(t), std::get<Is>(tup)...);
    }

    template <typename...Fns, typename T>
    constexpr static decltype(auto) call_helper(std::tuple<Fns...> tup, T&& t)
    {
      return call_helper_tuple(
          std::forward<T>(t),
          std::forward<decltype(tup)>(tup),
          std::make_index_sequence<sizeof...(Fns)>());
    }

    template <typename T>
    constexpr decltype(auto) operator()(T&& t) const
    {
      return call_helper(fs, std::forward<T>(t));
    }
  };

  template <typename... Fns>
  constexpr composed<Fns...> compose(Fns&&... fns)
  {
    return composed<Fns...>(std::forward<Fns>(fns)...);
  }

}

#endif
