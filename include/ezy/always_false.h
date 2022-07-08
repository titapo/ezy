#ifndef EZY_ALWAYS_FALSE_H_INCLUDED
#define EZY_ALWAYS_FALSE_H_INCLUDED

namespace ezy
{
  template <typename...>
  constexpr auto always_false = false;
}

#endif
