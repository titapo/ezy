#ifndef EZY_FEATURES_INVOCABLE_INCLUDED
#define EZY_FEATURES_INVOCABLE_INCLUDED

#include "../feature.h"
#include "../invoke.h"

namespace ezy
{
namespace features
{
  template <typename T>
  struct invocable : feature<T, invocable>
  {
    struct impl
    {
      template <typename ST, typename... Ts>
      static constexpr decltype(auto) invoke(ST&& t, Ts&&... ts)
      {
        return ezy::invoke(std::forward<ST>(t).get(), std::forward<Ts>(ts)...);
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
}

#endif
