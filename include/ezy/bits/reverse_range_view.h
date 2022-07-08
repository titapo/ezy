#ifndef EZY_BITS_REVERSE_RANGE_VIEW_H_INCLUDED
#define EZY_BITS_REVERSE_RANGE_VIEW_H_INCLUDED

#include <ezy/bits/range_utils.h>
#include <ezy/experimental/keeper.h>

namespace ezy::detail
{
  template <typename Keeper>
  struct reverse_range_view
  {
    using Range = ezy::experimental::keeper_value_type_t<Keeper>;
    using orig_it_category = typename std::iterator_traits<iterator_type_t<Range>>::iterator_category;
    static_assert(std::is_base_of_v<std::bidirectional_iterator_tag, orig_it_category>);

    auto begin() const
    {
      using std::crbegin;
      return crbegin(range.get());
    }

    auto end() const
    {
      using std::crend;
      return crend(range.get());
    }

    auto begin()
    {
      using std::rbegin;
      return rbegin(range.get());
    }

    auto end()
    {
      using std::rbegin;
      return rend(range.get());
    }

    Keeper range;
  };
}

#endif
