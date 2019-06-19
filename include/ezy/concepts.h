#ifndef EZY_CONCEPTS_H_INCLUDED
#define EZY_CONCEPTS_H_INCLUDED

namespace ezy
{
  /**
   * Example use:
   *
   * template <typename T, Requires<trait_constraint_v<T>> = true>
   * auto f(T&& t);
   *
   * (Borrowed from: C++ Concepts and Ranges - Mateusz Pusz - Meeting C++ 2018 )
   */
  template <bool B>
  using Requires = std::enable_if_t<B, bool>;
}

#endif
