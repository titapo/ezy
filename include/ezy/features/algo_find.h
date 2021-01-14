#ifndef EZY_FEATURES_ALGO_FIND_H_INCLUDED
#define EZY_FEATURES_ALGO_FIND_H_INCLUDED

#include <ezy/bits/find.h>

namespace ezy::features
{
  struct algo_find
  {
    template <typename T>
    struct impl
    {
      // find
      template <typename Element>
      auto find(Element&& element) const &
      {
        return ezy::find(static_cast<const T&>(*this).get(), std::forward<Element>(element));
      }

      template <typename Element>
      auto find(Element&& element) &
      {
        return ezy::find(static_cast<T&>(*this).get(), std::forward<Element>(element));
      }

      template <typename Element>
      auto find(Element&& element) &&
      {
        return ezy::find(static_cast<T&&>(*this).get(), std::forward<Element>(element));
      }

      // find_if
      template <typename Predicate>
      auto find_if(Predicate&& predicate) const &
      {
        return ezy::find_if(static_cast<const T&>(*this).get(), std::forward<Predicate>(predicate));
      }

      template <typename Predicate>
      auto find_if(Predicate&& predicate) &
      {
        return ezy::find_if(static_cast<T&>(*this).get(), std::forward<Predicate>(predicate));
      }

      template <typename Predicate>
      auto find_if(Predicate&& predicate) &&
      {
        return ezy::find_if(static_cast<T&&>(*this).get(), std::forward<Predicate>(predicate));
      }
    };
  };
}

#endif
