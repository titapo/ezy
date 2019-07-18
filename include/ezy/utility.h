#ifndef EZY_UTILITY_H_INCLUDED
#define EZY_UTILITY_H_INCLUDED

#include "strong_type.h"
#include "features/result_interface.h"
#include "features/visitable.h"

namespace ezy
{
  namespace features
  {
    template <typename T>
    struct inherit_std_optional : crtp<T, inherit_std_optional>
    {
      /* operator-> */
      constexpr decltype(auto) operator->() const
      { return (*this).that().get().operator->(); }

      constexpr decltype(auto) operator->()
      { return (*this).that().get().operator->(); }

      /* operator* */
      constexpr decltype(auto) operator*() const &
      { return (*this).that().get().operator*(); }

      constexpr decltype(auto) operator*() &
      { return (*this).that().get().operator*(); }

      constexpr decltype(auto) operator*() &&
      { return std::move(*this).that().get().operator*(); }

      constexpr decltype(auto) operator*() const &&
      { return std::move(*this).that().get().operator*(); }

      /* operator bool */
      constexpr explicit operator bool() const noexcept
      { return static_cast<bool>((*this).that().get()); }

      /* has_value() */
      constexpr bool has_value() const noexcept
      { return (*this).that().get().has_value(); }

      /* value() */
      constexpr decltype(auto) value() &
      { return (*this).that().get().value(); }

      constexpr decltype(auto) value() const &
      { return (*this).that().get().value(); }

      constexpr decltype(auto) value() &&
      { return std::move(*this).that().get().value(); }

      constexpr decltype(auto) value() const &&
      { return std::move(*this).that().get().value(); }

      /* value_or() */
      template <typename U>
      constexpr decltype(auto) value_or(U&& default_value) const &
      { return (*this).that().get().value_or(std::forward<U>(default_value)); }

      template <typename U>
      constexpr decltype(auto) value_or(U&& default_value) &&
      { return std::move(*this).that().get().value_or(std::forward<U>(default_value)); }

      // TODO consider to add
      // operator=
      // swap (alongside std::swap specialization)
      // reset
      // emplace
    };

  }

  template <typename T>
  using optional = strong_type<std::optional<T>, notag_t,
        result_interface<optional_adapter>::continuation,
        features::inherit_std_optional
      >;

  template <typename... Ts>
  using variant = strong_type<std::variant<Ts...>, notag_t, visitable>;

  template <typename Success, typename Error>
  using result = strong_type<std::variant<Success, Error>, notag_t, visitable, result_like_continuation>;
}

#endif
