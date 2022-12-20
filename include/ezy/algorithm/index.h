#ifndef EZY_ALGORITHM_INDEX_H_INCLUDED
#define EZY_ALGORITHM_INDEX_H_INCLUDED

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

  /**
   * Returns an element from `range` at specified location `index`.
   *
   * Internally it invokes `operator[]` or counts the indexed element by iterators.
   *
   * NOTE Currently there is no support for associative containers, like `std::map`.
   */
  template <typename Range>
  constexpr decltype(auto) index(Range&& range, detail::size_type_t<Range> index)
  {
    return detail::index_impl(std::forward<Range>(range), index, detail::priority_tag<1>{});
  }
}


#endif
