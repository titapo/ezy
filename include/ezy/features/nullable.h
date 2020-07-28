#ifndef EZY_FEATURES_NULLABLE_H_INCLUDED
#define EZY_FEATURES_NULLABLE_H_INCLUDED

#include "../feature.h"
#include "../strong_type_traits.h"

namespace ezy
{
namespace features
{
namespace experimental
{
  namespace detail
  {
    struct identity_fn
    {
      template <typename T>
      constexpr T&& operator()(T&& t) const noexcept
      {
        return std::forward<T>(t);
      }
    };

    struct dereference_fn
    {
      template <typename T>
        constexpr decltype(auto) operator()(T&& t) const noexcept
        {
          return *(std::forward<T>(t));
        }
    };

    template <typename T>
    struct default_ctor_of
    {
      using value_type = T;
      constexpr value_type operator()() const
      {
        return value_type{};
      }
    };

    template <typename T>
    struct default_ctor_of_plain_type : default_ctor_of<ezy::plain_type_t<T>> {};

    template <typename Unwrapper>
    struct nullable_unwrapper
    {
      using unwrapper_type = Unwrapper;

      template <typename T>
      struct apply : ezy::feature<T, apply>
      {
        using base = ezy::feature<T, apply>;
        using base::self;


        template <typename U>
        static constexpr decltype(auto) value_impl(U&& ult)
        {
          return Unwrapper{}(std::forward<U>(ult));
        }

        // if the underlying type has op* this should be used by default
        constexpr decltype(auto) value()
        {
          return value_impl(base::underlying());
        }

        constexpr decltype(auto) value() const
        {
          return value_impl(base::underlying());
        }
      };
    };

    template <typename NoneProvider, typename NoneChecker = std::equal_to<>>
    struct null_checker_with_binary_predicate
    {
      template <typename T>
      struct apply : ezy::feature<T, apply>
      {
        using none_provoder_type = NoneProvider;
        using none_checker_type = NoneChecker;

        using base = ezy::feature<T, apply>;
        using base::self;

        static constexpr auto make_null()
        {
          return NoneProvider{}();
        }

        template <typename U>
        static constexpr bool has_value_impl(const U& ult)
        {
          return !NoneChecker{}(ult, NoneProvider{}());
        }

        constexpr bool has_value() const
        {
          return has_value_impl(base::underlying());
        }

        static constexpr bool is_consistent()
        {
          return !T{make_null()}.has_value();
        }

      };
    };

    template <typename NoneChecker>
    struct null_checker_with_unary_predicate
    {
      template <typename T>
      struct apply : ezy::feature<T, apply>
      {
        using none_checker_type = NoneChecker;

        using base = ezy::feature<T, apply>;
        using base::self;

        template <typename U>
        static constexpr bool has_value_impl(const U& ult)
        {
          return !NoneChecker{}(ult);
        }

        constexpr bool has_value() const
        {
          return has_value_impl(base::underlying());
        }
      };
    };
  }

  /**
   * NoneProvider: type of a stateless callable, typically a function object, which returns a value representing
   * null. 
   * NoneChecker: type of a stateless binary predicate which returns true if the underlying value represents
   * none compared with the null value.
   *
   * Please note that `basic_nullable_as` feature does not affect object construction. A default constructed element
   * not necessarily contains a null value.
   */
  template <typename NoneProvider, typename NoneChecker = std::equal_to<>, typename Unwrapper = detail::identity_fn>
  struct basic_nullable_as
  {
    template <typename T>
    struct apply :
      detail::nullable_unwrapper<Unwrapper>::template apply<T>,
      detail::null_checker_with_binary_predicate<NoneProvider, NoneChecker>::template apply<T>
    {
      template <typename DefaultType>
      auto value_or(DefaultType def) const
      {
        if (this->has_value())
          return this->value();
        else
          return static_cast<decltype(this->value())>(def);
      }
    };
  };

  /**
   * NoneChecker: holds a callable object as a unary predicate. There is no value provider here, so
   * make_null() is not added here.
   */
  template <typename NoneChecker, typename Unwrapper = detail::identity_fn>
  struct nullable_if
  {
    template <typename T>
    struct apply :
      detail::nullable_unwrapper<Unwrapper>::template apply<T>,
      detail::null_checker_with_unary_predicate<NoneChecker>::template apply<T>
    {
      template <typename DefaultType>
      auto value_or(DefaultType def) const
      {
        if (this->has_value())
          return this->value();
        else
          return static_cast<decltype(this->value())>(def);
      }
    };
  };

  template <typename T>
  using default_nullable = typename basic_nullable_as<detail::default_ctor_of_plain_type<T>>::template apply<T>;

  template <typename T>
  using basic_nullable_ptr = typename basic_nullable_as<
    std::integral_constant<std::nullptr_t, nullptr>,
    std::equal_to<>,
    detail::dereference_fn
  >::template apply<T>;

}}}

#endif
