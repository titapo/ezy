#ifndef EZY_ALGORITHM_AT_H_INCLUDED
#define EZY_ALGORITHM_AT_H_INCLUDED

#include <ezy/bits/priority_tag.h>
#include <ezy/bits/range_utils.h>
#include <ezy/bits/optional_like_value.h>

namespace ezy
{
  namespace detail
  {
    template <typename Range>
    auto at_impl(Range&& range, detail::size_type_t<Range> index, priority_tag<0>) -> decltype(at(range, index))
    {
      return at(range, index);
    }

    template <typename Range>
    constexpr auto at_impl(Range&& range, detail::size_type_t<Range> index, priority_tag<1>) -> decltype(dependent_forward<Range>(range.at(index)))
    {
      return dependent_forward<Range>(range.at(index));
    }
  }

  template <typename Range>
  constexpr decltype(auto) at(Range&& range, detail::size_type_t<Range> index)
  {
    return detail::at_impl(std::forward<Range>(range), index, detail::priority_tag<1>{});
  }
}

#endif
