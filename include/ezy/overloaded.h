#ifndef EZY_OVERLOADED_H_INCLUDED
#define EZY_OVERLOADED_H_INCLUDED

#include <variant>

namespace ezy
{
  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

  struct tuple_overload
  {
    template <typename... Ts>
    auto operator()(Ts... ts)
    { return overloaded{std::forward<Ts>(ts)...}; }
  };
}

#endif
