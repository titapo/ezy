#ifndef EZY_FEATURES_ARITHMETIC_H_INCLUDED
#define EZY_FEATURES_ARITHMETIC_H_INCLUDED

#include "../feature.h"
#include "../strong_type_traits.h"
#include <experimental/type_traits>

namespace ezy
{
namespace features
{

  namespace detail
  {
    template <typename T>
    using operator_ne_t = decltype(std::declval<T>() != std::declval<T>());
  }

  template <typename T>
  struct addable : feature<T, addable>
  {
    using base = feature<T, addable>;
    using base::self;

    T operator+(T const& other) const { return T(self().get() + other.get()); }
    T& operator+=(T const& other)
    {
      self().get() += other.get();
      return self();
    }
  };

  template <typename T>
  struct subtractable : feature<T, subtractable>
  {
    using base = feature<T, subtractable>;
    using base::self;

    T operator-(T const& other) const { return T(self().get() - other.get()); }
    T& operator-=(T const& other)
    {
      self().get() -= other.get();
      return self();
    }
  };

  template <typename T>
  struct additive : addable<T>, subtractable<T> {};

  namespace detail
  {
    // !=
    template <typename T>
    constexpr bool not_equal(std::true_type, const T& lhs, const T& rhs)
    {
      return lhs != rhs;
    }

    // synthetize from ==
    template <typename T>
    constexpr bool not_equal(std::false_type, const T& lhs, const T& rhs)
    {
      return !(lhs == rhs);
    }
  }

  template <typename T>
  struct equal_comparable : feature<T, equal_comparable>
  {
    using base = feature<T, equal_comparable>;
    using base::self;

    friend constexpr bool operator==(const T& lhs, const T& rhs)
    {
      return lhs.get() == rhs.get();
    }

    friend constexpr bool operator!=(const T& lhs, const T& rhs)
    {
      return detail::not_equal(
          std::experimental::is_detected<ezy::features::detail::operator_ne_t, typename T::type>{},
          lhs.get(),
          rhs.get()
          );
    }
  };

  template <typename T>
  struct greater : feature<T, greater>
  {
    using base = feature<T, greater>;
    using base::self;

    friend bool operator>(const T& lhs, const T& rhs)
    {
      return lhs.get() > rhs.get();
    }
  };

  template <typename T>
  struct greater_equal : feature<T, greater_equal>
  {
    using base = feature<T, greater_equal>;
    using base::self;

    friend bool operator>=(const T& lhs, const T& rhs)
    {
      return rhs.get() >= rhs.get();
    }
  };

  template <typename T>
  struct less : feature<T, less>
  {
    using base = feature<T, less>;
    using base::self;

    friend bool operator<(const T& lhs, const T& rhs)
    {
      return lhs.get() < rhs.get();
    }
  };

  template <typename T>
  struct less_equal : feature<T, less_equal>
  {
    using base = feature<T, less_equal>;
    using base::self;

    friend bool operator<=(const T& lhs, const T& rhs)
    {
      return lhs.get() <= rhs.get();
    }
  };

  namespace detail
  {
    template <typename T>
    decltype(auto) to_plain_type(T&& t)
    {
      return static_cast<ezy::plain_type_t<std::remove_reference_t<T>>>(t); // forwarding?
    }

    template <typename T>
    decltype(auto) forward_plain_type(T&& t)
    {
      if constexpr (ezy::is_strong_type_v<ezy::remove_cvref_t<T>>)
      {
        return static_cast<T&&>(t).get();
      }
      else
      {
        return static_cast<T&&>(t);
      }
    }
  }

  // nicer solution?
  template <typename Multiplier, typename Result>
  struct multiplication_by_results
  {
    template <typename T>
    struct impl : feature<T, impl>
    {
      using base = feature<T, impl>;
      using base::self;

      friend Result operator*(const T& lhs, const Multiplier& rhs) { return Result(lhs.get() * detail::forward_plain_type(rhs)); }

      template <typename B = bool, typename = std::enable_if_t<!std::is_same<T, Multiplier>::value, B>>
      friend Result operator*(const Multiplier& lhs, const T& rhs) { return Result(detail::forward_plain_type(lhs) * rhs.get()); }

      // TODO conditionally constrain
      T& operator*=(const Multiplier& other)
      {
        self().get() *= detail::forward_plain_type(other);
        return self();
      }
    };
  };

  template <typename N>
  struct closed_multipliable_by
  {
    template <typename T>
    struct impl : feature<T, impl>
    {
      using base = feature<T, impl>;
      using base::self;

      friend T operator*(const T& lhs, const N& rhs) { return T(lhs.get() * detail::forward_plain_type(rhs)); }

      template <typename B = bool, typename = std::enable_if_t<!std::is_same<T, N>::value, B>>
      friend T operator*(const N& lhs, const T& rhs) { return T(detail::forward_plain_type(lhs) * rhs.get()); }

      T& operator*=(const N& other)
      {
        self().get() *= detail::forward_plain_type(other);
        return self();
      }
    };
  };

  template <typename T>
  using closed_multipliable = typename closed_multipliable_by<T>::template impl<T>;

  template <typename T>
  using multipliable_with_underlying = typename multiplication_by_results<ezy::extract_underlying_type_t<T>, T>::template impl<T>;

  template <typename T>
  using multipliable = multipliable_with_underlying<T>;

  // division
  template <typename Divisor, typename Result>
  struct division_by_results
  {
    template <typename T>
    struct no_self_divisible {};

    template <typename T>
    struct self_divisible : ezy::feature<T, self_divisible>
    {
      using base = ezy::feature<T, self_divisible>;
      using base::self;

      T& operator/=(const Divisor& other)
      {
        self().get() /= other;
        return self();
      }
    };

    template <typename T>
    struct helper_selector :
      std::conditional<
        std::is_same<T, Result>::value,
        self_divisible<T>,
        no_self_divisible<T>
      >::type
    {};

    template <typename T>
    struct impl : ezy::feature<T, impl>, helper_selector<T>
    {
      using base = ezy::feature<T, impl>;
      using base::self;

      static constexpr bool is_closed = std::is_same_v<T, Result>;

      friend Result operator/(const T& lhs, const Divisor& other)
      { return Result(lhs.get() / ezy::features::detail::forward_plain_type(other)); }
    };
  };

  template <typename Divisor>
  struct closed_divisible_by
  {
    template <typename T>
    struct impl : feature<T, impl>
    {
      using base = feature<T, impl>;
      using base::self;

      using Result = T;

      friend Result operator/(const T& lhs, const Divisor& other)
      {
        return Result(lhs.get() / ezy::features::detail::forward_plain_type(other));
      }

      T& operator/=(const Divisor& other)
      {
        self().get() /= detail::forward_plain_type(other);
        return self();
      }
    };
  };

  template <typename T>
  using closed_divisible = typename closed_divisible_by<T>::template impl<T>;

  //scalar means underlying here
  template <typename T>
  using divisible_by_scalar = typename division_by_results<ezy::extract_underlying_type_t<T>, T>::template impl<T>;

  template <typename T>
  using division_results_scalar = typename division_by_results<T, ezy::extract_underlying_type_t<T>>::template impl<T>;

  template <typename T>
  struct divisible : divisible_by_scalar<T>, division_results_scalar<T>
  {};

  template <typename N>
  struct multiplicative_by
  {
    template <typename T>
    struct impl : closed_multipliable_by<N>::template impl<T>, closed_divisible_by<N>::template impl<T>
    {
    };
  };

  template <typename T>
  struct multiplicative : multipliable<T>, divisible<T> {};
}}

#endif
