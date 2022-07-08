#ifndef EZY_BITS_REVERSE_RANGE_VIEW_H_INCLUDED
#define EZY_BITS_REVERSE_RANGE_VIEW_H_INCLUDED

#include <ezy/range.h> // iterator_type_t

namespace ezy::detail
{

  template <typename Range> 
  struct iterator_category
  {
    using type = typename std::iterator_traits<detail::iterator_type_t<Range>>::iterator_category;
  };

  template <typename Range> 
  using iterator_category_t = typename iterator_category<Range>::type;

  template <typename Range, typename IterCat> 
  struct does_range_iterator_implement
  {
    static constexpr bool value = std::is_base_of_v<IterCat, iterator_category_t<Range>>;
  };

  template <typename Range, typename IterCat>
  constexpr bool does_range_iterator_implement_v = does_range_iterator_implement<Range, IterCat>::value;

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
