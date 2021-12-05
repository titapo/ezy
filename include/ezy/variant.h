#ifndef EZY_VARIANT_H_INCLUDED
#define EZY_VARIANT_H_INCLUDED

#include "strong_type.h"
#include "features/visitable.h"

namespace ezy
{
  template <typename... Ts>
  using variant = strong_type<std::variant<Ts...>, notag_t, features::visitable>;
}

#endif
