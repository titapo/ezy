#ifndef EZY_FEATURES_ITERABLE_H_INCLUDED
#define EZY_FEATURES_ITERABLE_H_INCLUDED

#include <algorithm>
#include <functional>

#include "../range.h"
#include "../algorithm.h"

#include "algo_find.h"
#include "algo_iterable.h"

namespace ezy::features
{
  struct has_iterator
  {
    template <typename T>
    struct impl
    {
      constexpr auto begin() { return std::begin(static_cast<T&>(*this).get()); }
      constexpr auto begin() const { return std::begin(static_cast<const T&>(*this).get()); }
      constexpr auto cbegin() { return std::cbegin(static_cast<T&>(*this).get()); }
      constexpr auto cbegin() const { return std::cbegin(static_cast<const T&>(*this).get()); }

      constexpr auto end() { return std::end(static_cast<T&>(*this).get()); }
      constexpr auto end() const { return std::end(static_cast<const T&>(*this).get()); }
      constexpr auto cend() { return std::cend(static_cast<T&>(*this).get()); }
      constexpr auto cend() const { return std::cend(static_cast<const T&>(*this).get()); }
    };
  };

  template <typename T>
  struct inherit_range_view_features
  {
    using const_iterator = typename T::const_iterator;
  };

  struct iterable
  {
    template <typename T>
    struct impl :
      has_iterator::impl<T>,
      algo_find::impl<T>,
      algo_iterable::impl<T>
    {};
  };

}

#endif
