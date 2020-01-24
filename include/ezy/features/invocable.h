#ifndef EZY_FEATURES_INVOCABLE_INCLUDED
#define EZY_FEATURES_INVOCABLE_INCLUDED

#include "../strong_type.h"

namespace ezy::features
{
  template <typename T>
  struct invocable : feature<T, invocable>
  {
    struct impl
    {
      template <typename ST, typename... Ts>
      static constexpr decltype(auto) invoke(ST&& t, Ts&&... ts)
      {
        return std::invoke(std::forward<ST>(t).get(), std::forward<Ts>(ts)...);
      }
    };

    template <typename... Ts>
    decltype(auto) operator()(Ts&&...ts) &
    {
      return impl::invoke((*this).self(), std::forward<Ts>(ts)...);
    }

    template <typename... Ts>
    decltype(auto) operator()(Ts&&...ts) const &
    {
      return impl::invoke((*this).self(), std::forward<Ts>(ts)...);
    }

    template <typename... Ts>
    decltype(auto) operator()(Ts&&...ts) &&
    {
      return impl::invoke(std::move(*this).self(), std::forward<Ts>(ts)...);
    }
  };
}

#endif
