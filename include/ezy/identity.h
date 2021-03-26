#ifndef EZY_IDENTITY_H_INCLUDED
#define EZY_IDENTITY_H_INCLUDED

namespace ezy
{
    struct identity_fn
    {
      template <typename T>
      constexpr T&& operator()(T&& t) const noexcept
      {
        return static_cast<T&&>(t);
      }
    };

  static constexpr identity_fn identity{};
}

#endif
