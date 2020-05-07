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
      T operator*(numtype other) const { return T(self().get() * other); }
      T& operator*=(numtype other)
      {
        self().get() *= other;
        return self();
      }
    };
  };

  template <typename N>
  struct divisible_by
  {
    template <typename T>
    struct internal : feature<T, internal>
    {
      using base = feature<T, internal>;
      using base::self;

      using numtype = N;
      T operator/(numtype other) const { return T(self().get() / other); }
      T& operator/=(numtype other)
      {
        self().get() /= other;
        return self();
      }
    };
  };

  template <typename T>
  using divisible_by_int = divisible_by<int>::internal<T>;

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

}

#endif
