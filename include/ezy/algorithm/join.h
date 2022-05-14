#ifndef EZY_ALGORITHM_JOIN_H_INCLUDED
#define EZY_ALGORITHM_JOIN_H_INCLUDED

#include <ezy/bits/empty_size.h>
#include <ezy/range.h>
#include <algorithm>

namespace ezy
{
  template <typename ReturnType, typename Range, typename Separator = detail::value_type_t<Range>>
  constexpr ReturnType join(Range&& range, Separator&& separator = Separator{})
  {
    using std::begin;
    using std::end;
    if (ezy::empty(range))
    {
      return ReturnType{};
    }
    else
    {
      ReturnType result{*begin(range)};
      std::for_each(
          ++begin(range), // std::next() would be better
          end(range),
          [&result,separator](const auto& e)
          {
            result += separator;
            result += e;
          }
        );
      return result;
    }
  }

  template <typename Range, typename Separator = detail::value_type_t<Range>>
  constexpr auto join(Range&& range, Separator&& separator = Separator{})
  {
    using std::begin;
    using std::end;
    using ValueType = detail::value_type_t<Range>;
    return join<ValueType>(std::forward<Range>(range), std::forward<Separator>(separator));
  }
}

#endif
