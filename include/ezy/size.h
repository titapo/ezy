#ifndef EZY_BITS_SIZE_H_INCLUDED
#define EZY_BITS_SIZE_H_INCLUDED

#include "bits/priority_tag.h"
#include <iterator>

namespace ezy
{
  namespace detail
  {
    template <typename T>
    constexpr auto impl_size(const T& t, priority_tag<0>)
    {
      using std::begin;
      using std::end;
      return std::distance(begin(t), end(t));
    }

    template <typename T, std::size_t N>
    constexpr std::size_t impl_size(const T (&t)[N], priority_tag<1>)
    {
      return N;
    }

    template <typename T>
    constexpr auto impl_size(const T& t, priority_tag<2>) -> decltype(size(t))
    {
      return size(t);
    }

    template <typename T>
    constexpr auto impl_size(const T& t, priority_tag<3>) -> decltype(t.size())
    {
      return t.size();
    }
  }

  struct size_fn
  {
    template <typename T>
    [[nodiscard]] constexpr auto operator()(const T& t) const
    {
      return detail::impl_size(t, detail::priority_tag<3>{});
    }
  };

  inline namespace cpo
  {
    static constexpr size_fn size = {};
  }
}

#endif

