#ifndef EZY_STRING_H_INCLUDED
#define EZY_STRING_H_INCLUDED

#include <string>

namespace ezy
{
  struct to_string_fn
  {
    template <typename T>
    constexpr auto operator()(T&& t) const -> decltype(std::to_string(t))
    {
      return std::to_string(std::forward<T>(t));
    }

    template <typename T>
    constexpr auto operator()(T&& t) const -> decltype(to_string(std::forward<T>(t)))
    {
      return to_string(std::forward<T>(t));
    }

    /**
     * This overload forwards the call to std::string. It allows `ezy::to_string` to be called for char*,
     * which is convenient.
     *
     * NOTE: It can call std::string copy and move constructors too, so with inadvertent use it can
     * cause superfluous copy or move.
     */
    template <typename T>
    constexpr auto operator()(T&& t) const -> decltype(std::string(std::forward<T>(t)))
    {
      return std::string(std::forward<T>(t));
    }
  };

  static constexpr to_string_fn to_string{};
}

#endif
