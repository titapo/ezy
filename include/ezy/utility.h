#ifndef EZY_UTILITY_H_INCLUDED
#define EZY_UTILITY_H_INCLUDED

#include "strong_type.h"
#include "features/result_interface.h"
#include "features/visitable.h"

namespace ezy::features
{
  template <typename... Ts>
  using variant = strong_type<std::variant<Ts...>, notag_t, features::visitable>;

  template <typename Success, typename Error>
  using result = strong_type<std::variant<Success, Error>, notag_t, features::visitable, features::result_like_continuation>;
}

#endif
