#ifndef EZY_FEATURES_ARITHMETIC_H_INCLUDED
#define EZY_FEATURES_ARITHMETIC_H_INCLUDED

#include "../feature.h"
#include <experimental/type_traits>

namespace ezy::features
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

  template <typename T>
  struct equal_comparable : feature<T, equal_comparable>
  {
    using base = feature<T, equal_comparable>;
    using base::self;

    constexpr bool operator==(const T& rhs) const
    {
      return self().get() == rhs.get();
    }

    constexpr bool operator!=(const T& rhs) const
    {
      if constexpr (std::experimental::is_detected_v<ezy::features::detail::operator_ne_t, typename T::type>)
      {
        return self().get() != rhs.get();
      }
      else
      {
        return !(self() == rhs);
      }
    }
  };

  template <typename T>
  struct greater : feature<T, greater>
  {
    using base = feature<T, greater>;
    using base::self;

    bool operator>(const T& rhs) const
    {
      return self().get() > rhs.get();
    }
  };

  template <typename T>
  struct greater_equal : feature<T, greater_equal>
  {
    using base = feature<T, greater_equal>;
    using base::self;

    bool operator>=(const T& rhs) const
    {
      return self().get() >= rhs.get();
    }
  };

  template <typename T>
  struct less : feature<T, less>
  {
    using base = feature<T, less>;
    using base::self;

    bool operator<(const T& rhs) const
    {
      return self().get() < rhs.get();
    }
  };

  template <typename T>
  struct less_equal : feature<T, less_equal>
  {
    using base = feature<T, less_equal>;
    using base::self;

    bool operator<=(const T& rhs) const
    {
      return self().get() <= rhs.get();
    }
  };

  namespace detail
  {
    template <typename T>
    decltype(auto) to_plain_type(T&& t)
    {
      return static_cast<ezy::plain_type_t<std::remove_reference_t<T>>>(t); // forwarding?
    }
  }

  // nicer solution?
  template <typename N>
  struct multipliable_by
  {
    template <typename T>
    struct internal : feature<T, internal>
    {
      using base = feature<T, internal>;
      using base::self;

      using numtype = N;
      friend T operator*(const T& lhs, numtype rhs) { return T(lhs.get() * detail::to_plain_type(rhs)); }

      template <typename B = bool, typename = std::enable_if_t<!std::is_same_v<T, N>, B>>
      friend T operator*(numtype lhs, const T& rhs) { return T(detail::to_plain_type(lhs) * rhs.get()); }

      T& operator*=(numtype other)
      {
        self().get() *= detail::to_plain_type(other);
        return self();
      }
    };
  };

  template <typename T>
  using multipliable = typename multipliable_by<T>::template internal<T>;

  template <typename T>
  using multipliable_with_underlying = typename multipliable_by<ezy::extract_underlying_type_t<T>>::template internal<T>;

  template <typename N>
  struct divisible_by
  {
    template <typename T>
    struct internal : feature<T, internal>
    {
      using base = feature<T, internal>;
      using base::self;

      using numtype = N;
      T operator/(numtype other) const { return T(self().get() / detail::to_plain_type(other)); }
      T& operator/=(numtype other)
      {
        self().get() /= detail::to_plain_type(other);
        return self();
      }
    };
  };

  template <typename T>
  using divisible = typename divisible_by<T>::template internal<T>;

  template <typename T>
  using divisible_by_int = divisible_by<int>::internal<T>;

  template <typename T>
  using divisible_with_underlying = typename divisible_by<ezy::extract_underlying_type_t<T>>::template internal<T>;

  template <typename N>
  struct multiplicative_by
  {
    template <typename T>
    struct internal : multipliable_by<N>::template internal<T>, divisible_by<N>::template internal<T>
    {
      //using numtype = N;
    };
  };

  template <typename T>
  using multiplicative_by_int = multiplicative_by<int>::internal<T>;

  template <typename T>
  struct multiplicative : multipliable<T>, divisible<T> {};

}

#endif
