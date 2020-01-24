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
    struct operator_arrow : feature<T, operator_arrow>
    {
      using base = feature<T, operator_arrow>;

      /* operator-> */
      constexpr decltype(auto) operator->() const
      { return (*this).underlying().operator->(); }

      constexpr decltype(auto) operator->()
      { return (*this).underlying().operator->(); }
    };

    template <typename T>
    struct operator_star : feature<T, operator_star>
    {
      using base = feature<T, operator_star>;

      /* operator* */
      constexpr decltype(auto) operator*() const &
      { return (*this).underlying().operator*(); }

      constexpr decltype(auto) operator*() &
      { return (*this).underlying().operator*(); }

      constexpr decltype(auto) operator*() &&
      { return std::move(*this).underlying().operator*(); }

      constexpr decltype(auto) operator*() const &&
      { return std::move(*this).underlying().operator*(); }

    };

    namespace detail
    {
      template <typename T>
      struct inherit_std_optional_helper : feature<T, inherit_std_optional_helper>
      {
        using base = feature<T, inherit_std_optional_helper>;

        /* operator bool */
        constexpr explicit operator bool() const noexcept
        { return static_cast<bool>((*this).underlying()); }

        /* has_value() */
        constexpr bool has_value() const noexcept
        { return (*this).underlying().has_value(); }

        /* value() */
        constexpr decltype(auto) value() &
        { return (*this).underlying().value(); }

        constexpr decltype(auto) value() const &
        { return (*this).underlying().value(); }

        constexpr decltype(auto) value() &&
        { return std::move(*this).underlying().value(); }

        constexpr decltype(auto) value() const &&
        { return std::move(*this).underlying().value(); }

        /* value_or() */
        template <typename U>
        constexpr decltype(auto) value_or(U&& default_value) const &
        { return (*this).underlying().value_or(std::forward<U>(default_value)); }

        template <typename U>
        constexpr decltype(auto) value_or(U&& default_value) &&
        { return std::move(*this).underlying().value_or(std::forward<U>(default_value)); }

        // TODO consider to add
        // operator=
        // swap (alongside std::swap specialization)
        // reset
        // emplace
      };
    }

    template <typename T>
    struct inherit_std_optional :
      detail::inherit_std_optional_helper<T>,
      operator_arrow<T>,
      operator_star<T>
    {};

  }

  template <typename T>
  using optional = strong_type<std::optional<T>, notag_t,
        features::result_interface<features::optional_adapter>::continuation,
        features::inherit_std_optional
      >;

  template <typename... Ts>
  using variant = strong_type<std::variant<Ts...>, notag_t, features::visitable>;

  template <typename Success, typename Error>
  using result = strong_type<std::variant<Success, Error>, notag_t, features::visitable, features::result_like_continuation>;
}

#endif
