#ifndef EZ_BITS_RANGE_UTILS_H_INCLUDED
#define EZ_BITS_RANGE_UTILS_H_INCLUDED

#include <ezy/bits/empty_size.h> // ezy::size

namespace ezy
{
namespace detail
{
  template <typename T>
  struct const_iterator_type
  {
    using type = decltype(std::begin(std::declval<const T>()));
  };

  template <typename T>
  using const_iterator_type_t = typename const_iterator_type<T>::type;

  template <typename T>
  struct iterator_type
  {
    using type = decltype(std::begin(std::declval<T&>()));
  };

  template <typename T>
  using iterator_type_t = typename iterator_type<T>::type;

  template <typename Range>
  struct iterator_category
  {
    using type = typename std::iterator_traits<detail::iterator_type_t<Range>>::iterator_category;
  };

  template <typename Range>
  using iterator_category_t = typename iterator_category<Range>::type;

  template <typename Range, typename IterCat>
  struct does_range_iterator_implement
  {
    static constexpr bool value = std::is_base_of_v<IterCat, iterator_category_t<Range>>;
  };

  template <typename Range, typename IterCat>
  constexpr bool does_range_iterator_implement_v = does_range_iterator_implement<Range, IterCat>::value;


  template <typename T>
  struct value_type
  {
    using type = ezy::remove_cvref_t<decltype(*std::begin(std::declval<T>()))>;
  };

  template <typename T>
  using value_type_t = typename value_type<T>::type;

  template <typename T>
  struct const_value_type
  {
    using type = std::remove_reference_t<decltype(*std::begin(std::declval<const T>()))>;
  };

  template <typename T>
  using const_value_type_t = typename const_value_type<T>::type;

  template <typename T, typename = void>
  struct size_type
  {
    using type = decltype(ezy::size(std::declval<T>()));
  };

  template <typename T>
  struct size_type<T, void_t<typename ezy::remove_cvref_t<T>::size_type>>
  {
    using type = typename ezy::remove_cvref_t<T>::size_type;
  };

  template <typename T>
  using size_type_t = typename size_type<T>::type;
}
}


#endif
