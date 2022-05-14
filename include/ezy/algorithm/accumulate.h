#ifndef EZY_ALGORITHM_ACCUMULATE_H_INCLUDED
#define EZY_ALGORITHM_ACCUMULATE_H_INCLUDED

#include <ezy/invoke.h>

namespace ezy
{
  namespace detail
  {
    // accumulate polyfill brings c++20 (constexpr, move) features to c++14
    template <typename ItFirst, typename ItLast, typename T>
    constexpr T accumulate(ItFirst first, ItLast last, T init)
    {
      for (; first != last; ++first)
      {
        init = std::move(init) + *first;
      }
      return init;
    }

    template <typename ItFirst, typename ItLast, typename T, typename BinaryOp>
    constexpr T accumulate(ItFirst first, ItLast last, T init, BinaryOp op)
    {
      for (; first != last; ++first)
      {
        init = ezy::invoke(op, std::move(init), *first);
      }
      return init;
    }
  }

  template <typename Range, typename Init>
  constexpr Init accumulate(Range&& range, Init&& init)
  {
    return detail::accumulate(std::begin(range), std::end(range), std::forward<Init>(init));
  }

  template <typename Range, typename Init, typename BinaryOp>
  constexpr Init accumulate(Range&& range, Init&& init, BinaryOp&& op)
  {
    return detail::accumulate(std::begin(range), std::end(range), std::forward<Init>(init), std::forward<BinaryOp>(op));
  }
}

#endif
