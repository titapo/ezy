#ifndef EZY_EXPERIMENTAL_FUNCTION_HH
#define EZY_EXPERIMENTAL_FUNCTION_HH

#include <functional>

namespace ezy::experimental::function
{

  /**
   * curry
   */
  template <typename T, typename U, typename F, typename = std::is_invocable<F(T, T)>>
  auto curry(F&& f)
  {
    // TODO accept different references
    return [=](T a) {
      return [=](U b) {
        return f(a, b);
      };
    };
  }

  /**
   * compose
   */
  template <typename F1, typename F2>
  struct composed
  {
    composed(F1 f1, F2 f2)
      : fs(f1, f2)
    {}

    template <typename T>
    auto operator()(T&& p)
    {
      return std::get<1>(fs)(std::get<0>(fs)(std::forward<T>(p)));
    }

    std::tuple<F1, F2> fs;
  };

  //template <typename F1, typename F2> composed(F1, F2) -> composed<F1, F2>;

  template <typename F1, typename F2>
  composed<F1, F2> compose(F1&& f1, F2&& f2)
  {
    return composed<F1, F2>(f1, f2);
  }

}

#endif
