#ifndef EZY_TYPE_TRAITS_H_INCLUDED
#define EZY_TYPE_TRAITS_H_INCLUDED

#include <type_traits>

namespace ezy
{
  template <typename T>
  struct remove_cvref
  {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
  };

  template <typename T>
  using remove_cvref_t = typename remove_cvref<T>::type;
}

#endif
