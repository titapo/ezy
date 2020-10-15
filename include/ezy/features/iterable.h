#ifndef EZY_FEATURES_ITERABLE_H_INCLUDED
#define EZY_FEATURES_ITERABLE_H_INCLUDED

#include <algorithm>
#include <functional>

#include "../range.h"
#include "../algorithm.h"

#include "algo_find.h"
#include "algo_iterable.h"

#include <experimental/type_traits>

namespace ezy::features
{
  template <typename T>
  struct has_iterator : feature<T, has_iterator>
  {
    using base = feature<T, has_iterator>;

    constexpr auto begin() { return std::begin(base::underlying()); }
    constexpr auto begin() const { return std::begin(base::underlying()); }
    constexpr auto cbegin() { return std::cbegin(base::underlying()); }
    constexpr auto cbegin() const { return std::cbegin(base::underlying()); }

    constexpr auto end() { return std::end(base::underlying()); }
    constexpr auto end() const { return std::end(base::underlying()); }
    constexpr auto cend() { return std::cend(base::underlying()); }
    constexpr auto cend() const { return std::cend(base::underlying()); }
  };

  template <typename T>
  struct inherit_range_view_features : crtp<T, inherit_range_view_features>
  {
    using const_iterator = typename T::const_iterator;
  };

  template <typename T>
  struct iterable :
    has_iterator<T>,
    algo_find<T>,
    algo_iterable<T>
  {};

}

#endif
