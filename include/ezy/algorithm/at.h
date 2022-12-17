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

namespace ezy
{
  namespace detail
  {
    template <typename Range>
    decltype(auto) index_impl(Range&& range, detail::size_type_t<Range> index, priority_tag<0>)
    {
      using std::begin;
      return *std::next(begin(range), index);
    }

    template <typename Range>
    constexpr auto index_impl(Range&& range, detail::size_type_t<Range> index, priority_tag<1>) -> decltype(dependent_forward<Range>(range[index]))
    {
      return dependent_forward<Range>(range[index]);
    }
  }

  template <typename Range>
  constexpr decltype(auto) index(Range&& range, detail::size_type_t<Range> index)
  {
    return detail::index_impl(std::forward<Range>(range), index, detail::priority_tag<1>{});
  }
}

namespace ezy
{
  namespace detail
  {
    template <typename Range>
    auto checked_index_impl(Range&& range, detail::size_type_t<Range> index, priority_tag<0>) -> decltype(detail::optional_like_value_type_t<Range>{})
    {
      using std::begin;
      using std::end;

      using DifferenceType = typename iterator_type_t<Range>::difference_type;

      auto first = begin(range);
      if (std::distance(first, end(range)) <= static_cast<DifferenceType>(index))
      {
        return make_optional_like_value<Range>();
      }
        return make_optional_like_value<Range>(*std::next(first, index));
    }

    template <typename Range>
    constexpr auto checked_index_impl(Range&& range, detail::size_type_t<Range> index, priority_tag<1>) -> decltype(detail::optional_like_value_type_t<Range>{range[index]})
    {
      if (ezy::size(range) <= index)
      {
        return make_optional_like_value<Range>();
      }
      else
      {
        return make_optional_like_value<Range>(range[index]);
      }
    }
  }

  template <typename Range>
  constexpr decltype(auto) checked_index(Range&& range, detail::size_type_t<Range> index)
  {
    return detail::checked_index_impl(std::forward<Range>(range), index, detail::priority_tag<1>{});
  }
}

#endif
