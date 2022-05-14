#ifndef EZY_BITS_TUPLE_H_INCLUDED
#define EZY_BITS_TUPLE_H_INCLUDED

#include <tuple>

namespace ezy
{
  struct make_tuple_fn
  {
    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
      return std::make_tuple(std::forward<Args>(args)...);
    }
  };

  inline constexpr make_tuple_fn make_tuple{};

  struct forward_as_tuple_fn
  {
    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
      return std::forward_as_tuple(std::forward<decltype(args)>(args)...);
    }
  };

  inline constexpr forward_as_tuple_fn forward_as_tuple{};

}

#endif
