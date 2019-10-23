#ifndef EZY_ALGORITHM_HH_INCLUDED
#define EZY_ALGORITHM_HH_INCLUDED

#include <type_traits>
#include <experimental/type_traits>

namespace ezy
{
  namespace detail
  {
    template <typename T>
    using empty_mem_fn_t = decltype(std::declval<T>().empty());

    template <typename Range>
    using size_mem_fn_t = decltype(std::declval<Range>().size());
  }

  template <typename T>
  [[nodiscard]] constexpr auto empty(const T& t)
  {
    if constexpr (std::experimental::is_detected<detail::empty_mem_fn_t, T>::value)
    {
      return t.empty();
    }
    else
    {
      using std::begin;
      using std::end;
      return !(begin(t) != end(t));
    }
  }

  template <typename T>
  [[nodiscard]] constexpr auto size(const T& t)
  {
    if constexpr (std::experimental::is_detected<detail::size_mem_fn_t, T>::value)
    {
      return t.size();
    }
    else
    {
      using std::begin;
      using std::end;
      return std::distance(begin(t), end(t));
    }
  }
}

#endif
