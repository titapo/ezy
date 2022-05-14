#ifndef EZY_ALGORITHM_RANGE_H_INCLUDED
#define EZY_ALGORITHM_RANGE_H_INCLUDED

#include <ezy/algorithm/iterate.h>
#include <ezy/algorithm/take.h>

namespace ezy
{
  template <typename T>
  constexpr auto range(const T& until)
  {
    return ezy::take_while(ezy::iterate(T{0}), [until](const T& e) { return e < until; });
  }

  template <typename T>
  constexpr auto range(const T& from, const T& until)
  {
    return ezy::take_while(ezy::iterate(from), [until](const T& e) { return e < until; });
  }

  namespace detail
  {
    template <typename T>
    struct increment_by
    {
      explicit increment_by(const T& t)
        : step{t}
      {}

      template <typename U>
      auto operator()(const U& u) const
      {
        return u + step;
      }

      T step{};
    };
  }

  template <typename T>
  constexpr auto range(const T& from, const T& until, const T& step)
  {
    using Type = ezy::remove_cvref_t<T>;
    const auto increment_by_step = detail::increment_by<Type>{step};
    const auto number_of_steps = ((until - from) + (step - 1)) / step;
    return ezy::take(ezy::iterate(from, increment_by_step), number_of_steps);
  }
}

#endif
