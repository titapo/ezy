#ifndef EZY_ALGORITHM_CHECKED_INDEX_H_INCLUDED
#define EZY_ALGORITHM_CHECKED_INDEX_H_INCLUDED

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

  /**
   * Returns an element from `range` at specified location `index`.
   * Works the same as `ezy::index`, but with an additional bounds checking. Therefore it returns an optional-like
   * object, which can be an (T is the value_type of the `range`)
   *  * ezy::pointer<T>: if the element can be referenced
   *  * ezy::optional<T>: if the range element cannot be referenced. Maybe the range itself is an rvalue, or the
   *  underlying operation returns the element by value, eg. this can be the case for `ezy::transform`-ed elements.
   *
   * Internally it invokes `operator[]` or counts the indexed element by iterators.
   *
   * NOTE Currently there is no support for associative containers, like `std::map`.
   */
  template <typename Range>
  constexpr decltype(auto) checked_index(Range&& range, detail::size_type_t<Range> index)
  {
    return detail::checked_index_impl(std::forward<Range>(range), index, detail::priority_tag<1>{});
  }
}


#endif
