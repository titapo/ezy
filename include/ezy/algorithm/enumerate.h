#ifndef EZY_ALGORITHM_ENUMERATE_H_INCLUDED
#define EZY_ALGORITHM_ENUMERATE_H_INCLUDED

#include <ezy/algorithm/iterate.h>
#include <ezy/algorithm/zip.h>
#include <ezy/range.h> // detail::size_type

namespace ezy
{
  /**
   * When multiple ranges passed, they has to have the same size_type.
   */
  template <typename Range0, typename... Ranges>
  constexpr auto enumerate(Range0&& range0, Ranges&&... ranges)
  {
    using SizeType = detail::size_type_t<Range0>;
    static_assert(std::conjunction_v<std::is_same<SizeType, detail::size_type_t<Ranges>>...>);
    return ezy::zip(ezy::iterate<SizeType>({}), std::forward<Range0>(range0), std::forward<Ranges>(ranges)...);
  }
}

#endif
