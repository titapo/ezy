#ifndef EZY_FEATURES_ARITHMETIC_H_INCLUDED
#define EZY_FEATURES_ARITHMETIC_H_INCLUDED

#include "../strong_type_traits.h"
#include "../bits/priority_tag.h"
#include "../type_traits.h"

namespace ezy
{
namespace features
{
  struct addable
  {
    template <typename T>
    struct impl
    {
      friend T operator+(T const& lhs, T const& rhs) { return T(lhs.get() + rhs.get()); }

      friend T& operator+=(T& lhs, T const& rhs)
      {
        lhs.get() += rhs.get();
        return lhs;
      }
    };
  };

  struct subtractable
  {
    template <typename T>
    struct impl
    {
      friend T operator-(T const& lhs, T const& rhs) { return T(lhs.get() - rhs.get()); }

      friend T& operator-=(T& lhs, T const& rhs)
      {
        lhs.get() -= rhs.get();
        return lhs;
      }
    };
  };

  struct additive
  {
    template <typename T>
    struct impl : addable::impl<T>, subtractable::impl<T> {};
  };

  namespace detail
  {
    // synthetize from ==
    template <typename T>
    constexpr bool not_equal(const T& lhs, const T& rhs, ezy::detail::priority_tag<0>)
    {
      return !(lhs == rhs);
    }

    // !=
    template <typename T>
    constexpr auto not_equal(const T& lhs, const T& rhs, ezy::detail::priority_tag<1>) -> decltype(lhs != rhs)
    {
      return lhs != rhs;
    }

  }

  struct equal_comparable
  {
    template <typename T>
    struct impl
    {
      friend constexpr bool operator==(const T& lhs, const T& rhs)
      {
        return lhs.get() == rhs.get();
      }

      friend constexpr bool operator!=(const T& lhs, const T& rhs)
      {
        return detail::not_equal(
            lhs.get(),
            rhs.get(),
            ezy::detail::priority_tag<1>{}
            );
      }
    };
  };

  struct greater
  {
    template <typename T>
    struct impl
    {
      friend bool operator>(const T& lhs, const T& rhs)
      {
        return lhs.get() > rhs.get();
      }
    };
  };

  struct greater_equal
  {
    template <typename T>
    struct impl
    {
      friend bool operator>=(const T& lhs, const T& rhs)
      {
        return rhs.get() >= rhs.get();
      }
    };
  };

  struct less
  {
    template <typename T>
    struct impl
    {
      friend bool operator<(const T& lhs, const T& rhs)
      {
        return lhs.get() < rhs.get();
      }
    };
  };

  struct less_equal
  {
    template <typename T>
    struct impl
    {
      friend bool operator<=(const T& lhs, const T& rhs)
      {
        return lhs.get() <= rhs.get();
      }
    };
  };

  namespace detail
  {
    template <typename T>
    decltype(auto) to_plain_type(T&& t)
    {
      return static_cast<ezy::plain_type_t<std::remove_reference_t<T>>>(t); // forwarding?
    }

    template <typename T>
    decltype(auto) forward_plain_type_helper(T&& t, std::true_type)
    {
      return static_cast<T&&>(t).get();
    }

    template <typename T>
    decltype(auto) forward_plain_type_helper(T&& t, std::false_type)
    {
      return static_cast<T&&>(t);
    }

    template <typename T>
    decltype(auto) forward_plain_type(T&& t)
    {
      return forward_plain_type_helper(static_cast<T&&>(t), ezy::is_strong_type<ezy::remove_cvref_t<T>>{});
    }
  }

  // nicer solution?
  template <typename Multiplier, typename Result>
  struct multiplication_by_results
  {
    template <typename T>
    struct impl
    {
      friend Result operator*(const T& lhs, const Multiplier& rhs) { return Result(lhs.get() * detail::forward_plain_type(rhs)); }

      template <typename B = bool, typename = std::enable_if_t<!std::is_same<T, Multiplier>::value, B>>
      friend Result operator*(const Multiplier& lhs, const T& rhs) { return Result(detail::forward_plain_type(lhs) * rhs.get()); }

      // TODO conditionally constrain
      friend T& operator*=(T& lhs, const Multiplier& rhs)
      {
        lhs.get() *= detail::forward_plain_type(rhs);
        return lhs;
      }
    };
  };

  template <typename N>
  struct closed_multipliable_by
  {
    template <typename T>
    struct impl
    {
      friend T operator*(const T& lhs, const N& rhs) { return T(lhs.get() * detail::forward_plain_type(rhs)); }

      template <typename B = bool, typename = std::enable_if_t<!std::is_same<T, N>::value, B>>
      friend T operator*(const N& lhs, const T& rhs) { return T(detail::forward_plain_type(lhs) * rhs.get()); }

      friend T& operator*=(T& lhs, const N& rhs)
      {
        lhs.get() *= detail::forward_plain_type(rhs);
        return lhs;
      }
    };
  };

  struct closed_multipliable
  {
    template <typename T>
    using impl = typename closed_multipliable_by<T>::template impl<T>;
  };

  struct multipliable_with_underlying
  {
    template <typename T>
    using impl = typename multiplication_by_results<ezy::extract_underlying_type_t<T>, T>::template impl<T>;
  };

  struct multipliable
  {
    template <typename T>
    using impl = multipliable_with_underlying::impl<T>;
  };

  // division
  template <typename Divisor, typename Result>
  struct division_by_results
  {
    template <typename T>
    struct no_self_divisible {};

    template <typename T>
    struct self_divisible
    {
      friend T& operator/=(T& lhs, const Divisor& rhs)
      {
        lhs.get() /= rhs;
        return lhs;
      }
    };

    template <typename T>
    struct helper_selector :
      ezy::conditional_t<
        std::is_same<T, Result>::value,
        self_divisible<T>,
        no_self_divisible<T>
      >
    {};

    template <typename T>
    struct impl : helper_selector<T>
    {
      friend Result operator/(const T& lhs, const Divisor& other)
      { return Result(lhs.get() / ezy::features::detail::forward_plain_type(other)); }
    };
  };

  template <typename Divisor>
  struct closed_divisible_by
  {
    template <typename T>
    struct impl
    {
      using Result = T;

      friend Result operator/(const T& lhs, const Divisor& other)
      {
        return Result(lhs.get() / ezy::features::detail::forward_plain_type(other));
      }

      friend T& operator/=(T& lhs, const Divisor& rhs)
      {
        lhs.get() /= detail::forward_plain_type(rhs);
        return lhs;
      }
    };
  };

  struct closed_divisible
  {
    template <typename T>
    using impl = typename closed_divisible_by<T>::template impl<T>;
  };

  //scalar means underlying here
  struct divisible_by_scalar
  {
    template <typename T>
    using impl = typename division_by_results<ezy::extract_underlying_type_t<T>, T>::template impl<T>;
  };

  struct division_results_scalar
  {
    template <typename T>
    using impl = typename division_by_results<T, ezy::extract_underlying_type_t<T>>::template impl<T>;
  };

  struct divisible
  {
    template <typename T>
    struct impl : divisible_by_scalar::impl<T>, division_results_scalar::impl<T>
    {};
  };

  template <typename N>
  struct multiplicative_by
  {
    template <typename T>
    struct impl : closed_multipliable_by<N>::template impl<T>, closed_divisible_by<N>::template impl<T>
    {
    };
  };

  struct multiplicative
  {
    template <typename T>
    struct impl : multipliable::impl<T>, divisible::impl<T> {};
  };

  struct negatable
  {
    template <typename T>
    struct impl
    {
      friend T operator-(const T& t)
      {
        return T{-t.get()};
      }
    };
  };

}}

#endif
