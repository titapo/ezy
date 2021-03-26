#ifndef EZY_DEREFERENCE_H_INCLUDED
#define EZY_DEREFERENCE_H_INCLUDED

namespace ezy
{
  struct dereference_fn
  {
    template <typename T>
      constexpr decltype(auto) operator()(T&& t) const noexcept
      {
        return *(static_cast<T&&>(t));
      }
  };

  static constexpr dereference_fn dereference{};
}

#endif
