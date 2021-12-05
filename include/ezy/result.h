#ifndef EZY_RESULT_H_INCLUDED
#define EZY_RESULT_H_INCLUDED

#include "strong_type.h"
#include "features/visitable.h"

#include "strong_type.h"
#include "features/std_variant.h"
#include "features/visitable.h"

namespace ezy
{
  template <typename Success, typename Error>
  using result = strong_type<std::variant<Success, Error>, notag_t, features::visitable, features::result_like_continuation>;
}

#endif
