#ifndef EZY_ALGORITHM_REVERSE_H_INCLUDED
#define EZY_ALGORITHM_REVERSE_H_INCLUDED

#include <ezy/bits/reverse_range_view.h>
#include <ezy/bits/empty_size.h> // FIXME always_false

namespace ezy
{
  template <typename Range>
  constexpr auto reverse(Range&& range)
  {
    if constexpr (detail::does_range_iterator_implement_v<Range, std::bidirectional_iterator_tag>)
    {
      return detail::reverse_range_view<experimental::detail::deduce_keeper_t<Range>>{
        ezy::experimental::make_keeper(std::forward<Range>(range)) 
      };
    }
    else
    {
      static_assert(ezy::always_false<Range, detail::iterator_category_t<Range>>, "Range iterator is not bidirectional");
    }
  }
}

#endif
