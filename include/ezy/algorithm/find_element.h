#ifndef EZY_ALGORITHM_FIND_ELEMENT_H_INCLUDED
#define EZY_ALGORITHM_FIND_ELEMENT_H_INCLUDED

#include <ezy/bits/priority_tag.h>
#include <ezy/experimental/keeper.h>
#include <algorithm>

namespace ezy
{
  namespace detail
  {
    template <typename Range, typename Needle>
    constexpr auto find_element_impl(Range&& range, Needle&& needle, priority_tag<0>)
    {
      using std::begin;
      using std::end;
      return std::find(begin(range), end(range), std::forward<Needle>(needle));
    }

    template <typename Range, typename Needle>
    constexpr auto find_element_impl(Range&& range, Needle&& needle, priority_tag<1>)
      -> decltype(range.find(std::forward<Needle>(needle)))
    {
      return range.find(std::forward<Needle>(needle));
    }
  }

  template <typename Range, typename Needle>
  constexpr auto find_element(Range&& range, Needle&& needle)
  {
    static_assert(std::is_same_v<
        ezy::experimental::detail::ownership_category_t<Range>,
        ezy::experimental::reference_category_tag
        >, "Range must be a reference! Cannot form an iterator to a temporary!");

    return detail::find_element_impl(std::forward<Range>(range), std::forward<Needle>(needle), detail::priority_tag<1>{});
  }

  template <typename Range, typename Predicate>
  /*constexpr*/ auto find_element_if(Range&& range, Predicate&& pred)
  {
    static_assert(std::is_same_v<
        ezy::experimental::detail::ownership_category_t<Range>,
        ezy::experimental::reference_category_tag
        >, "Range must be a reference! Cannot form an iterator to a temporary!");

    using std::begin;
    using std::end;
    return std::find_if(begin(range), end(range), std::forward<Predicate>(pred));
  }

}

#endif
