#ifndef EZY_POINTER_H_INCLUDED
#define EZY_POINTER_H_INCLUDED

#include "strong_type.h"
#include "features/nullable_result.h"

namespace ezy
{
  template <typename T, typename Tag = void>
  using pointer = ezy::strong_type<T*, Tag, ezy::features::experimental::nullable_ptr>;
}

#endif
