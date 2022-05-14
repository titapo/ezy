#ifndef EZY_ALGORITHM_ITERATE_H_INCLUDED
#define EZY_ALGORITHM_ITERATE_H_INCLUDED

#include <ezy/range.h>

namespace ezy
{
  template <typename T, typename Fn>
  constexpr auto iterate(T&& t, Fn&& fn)
  {
    return ezy::detail::iterate_view<T, Fn>{std::forward<T>(t), std::forward<Fn>(fn)};
  }

  namespace detail
  {
    struct increment
    {
      template <typename T>
      constexpr T operator()(T t) const
      {
        return ++t;
      }
    };
  }

  template <typename T>
  constexpr auto iterate(T&& t)
  {
    using Fn = detail::increment;
    return ezy::detail::iterate_view<T, Fn>{std::forward<T>(t), Fn{}};
  }
}

#endif
