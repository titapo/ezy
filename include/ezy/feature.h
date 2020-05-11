#ifndef EZY_FEATURE_H_INCLUDED
#define EZY_FEATURE_H_INCLUDED

#include <utility> // for std::move

namespace ezy
{
  template <typename T, template<typename> class crtp_type>
  struct crtp
  {
    using self_type = T; // derived can reference itself
    constexpr T& self() & { return static_cast<T&>(*this); }
    constexpr const T& self() const & { return static_cast<const T&>(*this); }
    constexpr T&& self() && { return static_cast<T&&>(*this); }
  };

  template <typename T, template<typename> class crtp_type>
  struct feature : crtp<T, crtp_type>
  {
    constexpr auto& underlying() & { return (*this).self().get(); }
    constexpr const auto& underlying() const & { return (*this).self().get(); }
    constexpr auto&& underlying() && { return std::move(*this).self().get(); }
  };

  template <typename T, template<typename...> class crtp_type>
  struct crtp_with_params
  {
    T& that() { return static_cast<T&>(*this); }
    const T& that() const { return static_cast<const T&>(*this); }
  };
}

#endif
