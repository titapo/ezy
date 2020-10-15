#ifndef EZY_FEATURES_ALGO_FIND_H_INCLUDED
#define EZY_FEATURES_ALGO_FIND_H_INCLUDED

#include <ezy/feature.h>
#include <ezy/bits/find.h>

namespace ezy::features
{
  template <typename T>
  struct algo_find : feature<T, algo_find>
  {
    using base = feature<T, algo_find>;

    template <typename Element>
    auto find(Element&& element) const
    {
      return ezy::find((*this).underlying(), std::forward<Element>(element));
    }

    template <typename Predicate>
    auto find_if(Predicate&& predicate) const
    {
      return ezy::find_if((*this).underlying(), std::forward<Predicate>(predicate));
    }
  };
}

#endif
