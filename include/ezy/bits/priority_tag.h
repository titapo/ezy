#ifndef EZY_BITS_PRIORITY_TAG_H_INCLUDED
#define EZY_BITS_PRIORITY_TAG_H_INCLUDED

#include <cstddef>

namespace ezy
{
  namespace detail
  {
    template <size_t I> struct priority_tag : priority_tag<I - 1> {};
    template <> struct priority_tag<0>{};
  }
}

#endif
