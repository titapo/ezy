#ifndef EZY_FEATURES_COMMON_H_INCLUDED
#define EZY_FEATURES_COMMON_H_INCLUDED

#include "../strong_type.h"

namespace ezy::features
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

  template <typename T>
  struct operator_subscript : feature<T, operator_subscript>
  {
    using base = feature<T, operator_subscript>;

    using size_type_local = size_t;// typename base::self_type::type::size_type;

    constexpr decltype(auto) operator[](size_type_local pos)
    { return base::underlying()[pos]; }

    constexpr decltype(auto) operator[](size_type_local pos) const
    { return base::underlying()[pos]; }
  };
}

namespace ezy::features
{
  // TODO std::ostream should not be named here
  /**
   * features
   */
  template <typename T>
  struct printable : feature<T, printable>
  {
    using base = feature<T, printable>;

    std::ostream& print_to_stream(std::ostream& ostr) const
    {
      return ostr << base::underlying();
    }
  };

  template <typename T, typename Tag, template <typename> class... Features>
  std::ostream& operator<<(std::ostream& ostr, const strong_type<T, Tag, Features...>& strong)
  {
    return strong.print_to_stream(ostr);
  }

  template <typename T>
  struct clonable : feature<T, clonable>
  {
    using base = feature<T, clonable>;

    T clone() const
    {
      return {base::underlying()};
    }
  };

  template <typename T>
  struct implicit_convertible : feature<T, implicit_convertible>
  {
    using base = feature<T, implicit_convertible>;
    using underlying_type = extract_underlying_type_t<T>;

    operator const underlying_type&() const
    {
      return base::underlying();
    }

    operator underlying_type&()
    {
      return base::underlying;
    }
  };
}



#endif
