#ifndef EZY_OPTIONAL_H_INCLUDED
#define EZY_OPTIONAL_H_INCLUDED

#include "features/std_optional.h"

namespace ezy
{
  template <typename T>
  using optional = strong_type<std::optional<T>, notag_t,
        features::result_interface<features::optional_adapter>,
        features::inherit_std_optional
      >;
}

#endif
