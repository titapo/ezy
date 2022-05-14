#ifndef EZY_ALGORITHM_COLLECT_H_INCLUDED
#define EZY_ALGORITHM_COLLECT_H_INCLUDED

#include <ezy/type_traits.h>

#include <array> // for std::[c]begin|end

namespace ezy
{
  template <typename Result, typename Range>
  constexpr auto collect(Range&& range)
  {
    using std::cbegin;
    using std::cend;
    return Result(cbegin(range), cend(range));
  }

  template <template <typename, typename ...> class ResultWrapper, typename Range>
  constexpr auto collect(Range&& range)
  {
    using std::begin;
    using ElementType = ezy::remove_cvref_t<decltype(*begin(range))>;
    return collect<ResultWrapper<ElementType>>(std::forward<Range>(range));
  }

  template <typename Range, typename OutputIter>
  OutputIter collect(Range&& range, OutputIter out)
  {
    for (auto&& e : range)
    {
      *out++ = e;
    }
    return out;
  }
}

#endif
