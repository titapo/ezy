#ifndef EZY_FEATURES_VISITABLE_H_INCLUDED
#define EZY_FEATURES_VISITABLE_H_INCLUDED

#include "../overloaded.h"
#include <variant>

namespace ezy::features
{
  template <typename T>
  struct visitable
  {
    template <typename... Visitors>
    decltype(auto) visit(Visitors&&... visitors) &
    {
      return std::visit(ezy::overloaded{std::forward<Visitors>(visitors)...}, static_cast<T&>(*this).get());
    }

    template <typename... Visitors>
    decltype(auto) visit(Visitors&&... visitors) const &
    {
      return std::visit(ezy::overloaded{std::forward<Visitors>(visitors)...}, static_cast<const T&>(*this).get());
    }

    // TODO check it
    template <typename... Visitors>
    decltype(auto) visit(Visitors&&... visitors) &&
    {
      return std::visit(ezy::overloaded{std::forward<Visitors>(visitors)...}, static_cast<T&&>(*this).get());
    }
  };
}

// TODO support for std::get?

#endif
