#ifndef EZY_ARITHMETIC_H_INCLUDED
#define EZY_ARITHMETIC_H_INCLUDED

namespace ezy
{
  namespace detail
  {
    template <typename T>
    struct less_than_fn
    {
      bool operator()(const T& lhs) const
      {
        return lhs < rhs;
      }

      T rhs;

    };
  }

  template <typename T>
  auto less_than(T&& rhs)
  {
    return detail::less_than_fn<T>{std::forward<T>(rhs)};
  }

  namespace detail
  {
    template <typename T>
    struct greater_than_fn
    {
      bool operator()(const T& lhs) const
      {
        return lhs > rhs;
      }

      T rhs;

    };
  }

  template <typename T>
  auto greater_than(T&& rhs)
  {
    return detail::greater_than_fn<T>{std::forward<T>(rhs)};
  }

  // TODO less equal, greater equal
}

#endif
