#ifndef EZY_EXPERIMENTAL_VALUE_PROVIDER_H_INCLUDED
#define EZY_EXPERIMENTAL_VALUE_PROVIDER_H_INCLUDED

namespace ezy::experimental
{
  template <auto N>
  struct value_provider
  {
    using value_type = decltype(N);

    constexpr value_type operator()() const
    {
      return N;
    }
  };
}

#endif
