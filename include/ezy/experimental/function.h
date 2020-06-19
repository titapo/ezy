#ifndef EZY_EXPERIMENTAL_FUNCTION_HH
#define EZY_EXPERIMENTAL_FUNCTION_HH

#include <functional>
#include <ezy/invoke.h>

namespace ezy::experimental::function
{
  /**
   * curry
   */
  template <typename T, typename U, typename F, typename = std::is_invocable<F(T, T)>>
  decltype(auto) curry(F&& f)
  {
    // TODO accept different references
    return [=](T a) {
      return [=](U b) {
        return std::invoke(f, a, b);
      };
    };
  }
}

#endif
