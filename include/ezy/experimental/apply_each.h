#ifndef EZY_EXPERIMENTAL_APPLY_EACH_H_INCLUDED
#define EZY_EXPERIMENTAL_APPLY_EACH_H_INCLUDED

namespace ezy::experimental
{
  template <typename Fn, typename Tuple>
  void apply_each(Fn&& fn, Tuple&& t)
  {
    std::apply([&fn](auto&&... e) { (fn(std::forward<decltype(e)>(e)), ...);}, std::forward<Tuple>(t));
  }
}

#endif
