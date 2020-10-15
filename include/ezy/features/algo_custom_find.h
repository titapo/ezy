#ifndef EZY_FEATURES_ALGO_CUSTOM_FIND_H_INCLUDED
#define EZY_FEATURES_ALGO_CUSTOM_FIND_H_INCLUDED

#include <ezy/feature.h>
#include <ezy/custom_find.h>

namespace ezy
{
  namespace features
  {
    template <typename ResultMaker>
    struct algo_custom_find
    {
      template <typename T>
      struct impl : ezy::feature<T, impl>
      {
        template <typename Element>
        auto find(Element&& element) const
        {
          return ezy::custom_find<ResultMaker>{}((*this).underlying(), std::forward<Element>(element));
        }

        template <typename Predicate>
        auto find_if(Predicate&& predicate) const
        {
          return ezy::custom_find_if<ResultMaker>{}((*this).underlying(), std::forward<Predicate>(predicate));
        }
      };
    };
  }
}

#endif
