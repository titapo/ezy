#ifndef EZY_CUSTOM_FIND_H_INCLUDED
#define EZY_CUSTOM_FIND_H_INCLUDED

#include "bits/algorithm.h"

namespace ezy
{
  /**
   * custom_find, custom_find_if
   *
   * usage:
   *
   * static constexpr auto find = ezy::custom_find<Maker>{};
   * ...
   * std::vector v{...};
   * find(v, 5);
   *
   * Where `Maker` wraps the result, and defined as a function object, like
   * (pseudocode):
   *
   * struct Maker
   * {
   *   template <typename Range, typename Iterator>
   *   ... operator()(Iterator it, Iterator last)
   *   {
   *      if (it != last)
   *        return Some(*it);
   *      else
   *        return None;
   *   }
   * };
   *
   * `Range` is a fully qualified type, so its value category can be used to determine the concrete return
   * type.
   */
  template <typename ResultMaker>
  struct custom_find
  {
      template <typename Range, typename Needle>
      /*constexpr*/ auto operator()(Range&& range, Needle&& needle) const
      {
        auto found = ezy::find_element(range, std::forward<Needle>(needle));
        return ResultMaker{}.template operator()<Range>(found, end(range));
      }
  };

  template <typename ResultMaker>
  struct custom_find_if
  {
      template <typename Range, typename Predicate>
      /*constexpr*/ auto operator()(Range&& range, Predicate&& predicate) const
      {
        auto found = ezy::find_element_if(range, std::forward<Predicate>(predicate));
        return ResultMaker{}.template operator()<Range>(found, end(range));
      }
  };
}

#endif
