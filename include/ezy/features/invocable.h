#ifndef EZY_FEATURES_INVOCABLE_INCLUDED
#define EZY_FEATURES_INVOCABLE_INCLUDED

#include "../invoke.h"

namespace ezy
{
namespace features
{
  template <typename T>
  struct invocable
  {
    template <typename... Ts>
    decltype(auto) operator()(Ts&&...ts) &
    {
      return ezy::invoke(static_cast<T&>(*this).get(), std::forward<Ts>(ts)...);
    }

    template <typename... Ts>
    decltype(auto) operator()(Ts&&...ts) const &
    {
      return ezy::invoke(static_cast<const T&>(*this).get(), std::forward<Ts>(ts)...);
    }

    template <typename... Ts>
    decltype(auto) operator()(Ts&&...ts) &&
    {
      return ezy::invoke(static_cast<T&&>(*this).get(), std::forward<Ts>(ts)...);
    }
  };
}
}

#endif
