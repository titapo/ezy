#ifndef EZY_FEATURES_VISITABLE_H_INCLUDED
#define EZY_FEATURES_VISITABLE_H_INCLUDED

#include "../strong_type.h"

#include "../overloaded.h"
#include <variant>

namespace ezy::features
{
  template <typename T>
  struct visitable : feature<T, visitable>
  {
    using base = feature<T, visitable>;

    template <typename... Visitors>
    decltype(auto) visit(Visitors&&... visitors) &
    {
      return std::visit(ezy::overloaded{std::forward<Visitors>(visitors)...}, base::underlying());
    }

    template <typename... Visitors>
    decltype(auto) visit(Visitors&&... visitors) const &
    {
      return std::visit(ezy::overloaded{std::forward<Visitors>(visitors)...}, base::underlying());
    }

    // TODO check it
    template <typename... Visitors>
    decltype(auto) visit(Visitors&&... visitors) &&
    {
      return std::visit(ezy::overloaded{std::forward<Visitors>(visitors)...}, std::move(*this).underlying());
    }
  };
}

// TODO support for std::get?

#endif
