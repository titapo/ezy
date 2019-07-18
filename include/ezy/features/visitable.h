#ifndef EZY_FEATURES_VISITABLE_H_INCLUDED
#define EZY_FEATURES_VISITABLE_H_INCLUDED

#include "../strong_type.h"

#include "../overloaded.h"
#include <variant>

namespace ezy::features
{
  template <typename T>
  struct visitable : crtp<T, visitable>
  {
    template <typename... Visitors>
    decltype(auto) visit(Visitors&&... visitors) &
    {
      return std::visit(ezy::overloaded{std::forward<Visitors>(visitors)...}, this->that().get());
    }

    template <typename... Visitors>
    decltype(auto) visit(Visitors&&... visitors) const &
    {
      return std::visit(ezy::overloaded{std::forward<Visitors>(visitors)...}, this->that().get());
    }

    // TODO check it
    template <typename... Visitors>
    decltype(auto) visit(Visitors&&... visitors) &&
    {
      return std::visit(ezy::overloaded{std::forward<Visitors>(visitors)...}, std::move(*this).that().get());
    }
  };
}

// TODO support for std::get?

#endif
