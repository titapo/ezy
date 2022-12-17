#ifndef EZY_FEATURES_COMMON_H_INCLUDED
#define EZY_FEATURES_COMMON_H_INCLUDED

#include "../strong_type_traits.h"

#include <cstddef>

namespace ezy
{
namespace features
{
  struct operator_arrow
  {
    template <typename T>
    struct impl
    {
      constexpr decltype(auto) operator->() const
      { return static_cast<const T&>(*this).get().operator->(); }

      constexpr decltype(auto) operator->()
      { return static_cast<T&>(*this).get().operator->(); }
    };
  };

  struct operator_star
  {
    template <typename T>
    struct impl
    {
      constexpr decltype(auto) operator*() const &
      { return static_cast<const T&>(*this).get().operator*(); }

      constexpr decltype(auto) operator*() &
      { return static_cast<T&>(*this).get().operator*(); }

      constexpr decltype(auto) operator*() &&
      { return static_cast<T&&>(*this).get().operator*(); }

      constexpr decltype(auto) operator*() const &&
      { return static_cast<const T&&>(*this).get().operator*(); }
    };
  };

  struct operator_subscript
  {
    template <typename T>
    struct impl
    {
      using size_type_local = size_t;// typename base::self_type::type::size_type;

      constexpr decltype(auto) operator[](size_type_local pos)
      { return static_cast<T&>(*this).get()[pos]; }

      constexpr decltype(auto) operator[](size_type_local pos) const
      { return static_cast<const T&>(*this).get()[pos]; }
    };
  };

  /**
   * features
   */

  // TODO specify order: a << b, b << a
  // TODO specify return type
  template <typename U>
  struct left_shiftable_with
  {
    template <typename T>
    struct impl
    {
      friend decltype(auto) operator<<(U& lhs, const T& rhs)
      {
        return lhs << rhs.get();
      }
    };
  };

  template <typename T>
  struct clonable
  {
    T clone() const
    {
      return T{static_cast<const T&>(*this).get()};
    }

    // TODO moving overload?
    // TODO noexcept/trivial
    // TODO is it required at all?
  };

  template <typename T>
  struct implicit_convertible
  {
    using underlying_type = extract_underlying_type_t<T>;

    operator const underlying_type&() const
    {
      return static_cast<const T&>(*this).get();
    }

    operator underlying_type&()
    {
      return static_cast<T&>(*this).get();
    }
  };

  struct resettable
  {
    template <typename T>
    struct impl
    {
      decltype(auto) reset() noexcept(noexcept(std::declval<T>().get().reset()))
      {
        return static_cast<T&>(*this).get().reset();
      }
    };
  };
}
}



#endif
