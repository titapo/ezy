#ifndef EZY_BITS_EMPTY_SIZE_H_INCLUDED
#define EZY_BITS_EMPTY_SIZE_H_INCLUDED

#include "bits/priority_tag.h"
#include <iterator>

namespace ezy
{
  namespace detail
  {
    template <typename T>
    constexpr auto impl_empty(const T& t, priority_tag<0>) -> bool
    {
      using std::begin;
      using std::end;
      return !(begin(t) != end(t));
    }

    template <typename T>
    constexpr auto impl_empty(const T& t, priority_tag<1>) -> decltype(t.size())
    {
      return t.size() != 0;
    }

    template <typename T>
    constexpr auto impl_empty(const T& t, priority_tag<2>) -> decltype(t.empty())
    {
      return t.empty();
    }

    template <typename T>
    constexpr auto impl_empty(const T& t, priority_tag<3>) -> decltype(empty(t))
    {
      return empty(t);
    }
  }

  struct empty_fn
  {
    template <typename T>
    [[nodiscard]] constexpr auto operator()(const T& t) const
    {
      return detail::impl_empty(t, detail::priority_tag<3>{});
    }
  };

  inline namespace cpo
  {
    static constexpr empty_fn empty = {};
  }
}

#endif
