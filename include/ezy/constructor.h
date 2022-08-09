#ifndef EZY_CONSTRUCTOR_H_INCLUDED
#define EZY_CONSTRUCTOR_H_INCLUDED

namespace ezy {

  template <typename T>
  struct constructor_t
  {
    using value_type = T;

    template <typename... Args>
    constexpr value_type operator()(Args&&... args) const noexcept(noexcept(value_type(std::forward<Args>(args)...)))
    {
      return value_type(std::forward<Args>(args)...);
    }
  };

  template <typename T>
  constexpr constructor_t<T> construct{};


  template <typename T>
  struct brace_constructor_t
  {
    using value_type = T;

    template <typename... Args>
    constexpr value_type operator()(Args&&... args) const noexcept(noexcept(value_type{std::forward<Args>(args)...}))
    {
      return value_type{std::forward<Args>(args)...};
    }
  };

  template <typename T>
  constexpr brace_constructor_t<T> brace_construct{};

}

#endif
