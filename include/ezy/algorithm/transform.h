#ifndef EZY_ALGORITHM_TRANSFROM_H_INCLUDED
#define EZY_ALGORITHM_TRANSFROM_H_INCLUDED

#include <ezy/range.h>

namespace ezy
{
  template <typename Range, typename UnaryFunction>
  constexpr decltype(auto) transform(Range&& range, UnaryFunction&& fn)
  {
    using result_range_type = detail::range_view<experimental::detail::deduce_keeper_t<Range>, UnaryFunction>;
    return result_range_type{
        ezy::experimental::make_keeper(std::forward<Range>(range)),
        std::forward<UnaryFunction>(fn)
      };
  }
}


#endif
