#ifndef EZY_EXPERIMENTAL_KEEPER_H_INCLUDED
#define EZY_EXPERIMENTAL_KEEPER_H_INCLUDED

#include <type_traits>
#include <utility> // forward
#include <functional> //invoke

namespace ezy::experimental
{
  namespace detail
  {
    struct disable_implicit_copy
    {
      disable_implicit_copy() = default;
      disable_implicit_copy(const disable_implicit_copy&) = delete;
      disable_implicit_copy& operator=(const disable_implicit_copy&) = delete;
      disable_implicit_copy(disable_implicit_copy&&) = default;
      disable_implicit_copy& operator=(disable_implicit_copy&&) = default;
    };
  }

  /**
   * Category tags
   */
  struct owner_category_tag {};
  struct reference_category_tag {};

  /**
   * basic keeper: a type which can either own or refer to an object. It helps to be explicit and catches errors
   * in compile time.
   * */
  template <typename CategoryTag, typename T>
  struct basic_keeper;

  /**
   * Represents an owner. It cannot be implicitly "casted" to reference.
   * - use `.ref()` to "borrow" a reference
   * - use `std::move()` to move
   * - use `.copy()` to copy
   * - use `.mutable_copy()` if current instance is `const`, but a mutable copy needed
   *
   * Note: there is not protection against use-after move
   */
  template <typename T>
  struct basic_keeper<owner_category_tag, T> : detail::disable_implicit_copy
  {
    static_assert(!std::is_reference_v<T>, "T must not be a reference. Rather set the category!");

    using category_tag = owner_category_tag;

    using value_type = std::remove_reference_t<T>;
    using reference = value_type&;
    using const_reference = const value_type&;

    value_type t;

    basic_keeper(T&& u)
      : t(std::forward<T>(u))
    {}

    reference get() &
    {
      return t;
    }

    const_reference get() const &
    {
      return t;
    }

    value_type&& get() &&
    {
      return std::move(t);
    }

    basic_keeper<reference_category_tag, T> ref() &
    {
      return basic_keeper<reference_category_tag, T>(get());
    }

    basic_keeper<owner_category_tag, T> copy() const &
    {
      return basic_keeper<owner_category_tag, T>(T{get()});
    }

    basic_keeper<owner_category_tag, std::remove_const_t<T>> mutable_copy() const &
    {
      using MutableT = std::remove_const_t<T>;
      return basic_keeper<owner_category_tag, MutableT>(MutableT{get()});
    }

    operator basic_keeper<owner_category_tag, const T>() &&
    {
      return basic_keeper<owner_category_tag, const T>{std::move(t)};
    }

    template <typename Fn>
    decltype(auto) apply(Fn&& fn) &
    {
      return std::invoke(std::forward<Fn>(fn), t);
    }

    template <typename Fn>
    decltype(auto) apply(Fn&& fn) &&
    {
      return std::invoke(std::forward<Fn>(fn), std::move(t));
    }

  };

  /**
   * It's a kind of reference wrapper.
   * Rebinding is currently not available
   * */
  template <typename T>
  struct basic_keeper<reference_category_tag, T>
  {
    static_assert(!std::is_reference_v<T>, "T must not be a reference. Rather set the category!");

    using category_tag = reference_category_tag;

    using value_type = std::remove_reference_t<T>;
    using reference = value_type&;
    using const_reference = value_type&;

    reference t;

    explicit basic_keeper(T& u)
      : t(u)
    {}

    reference get()
    {
      return t;
    }

    const_reference get() const
    {
      return t;
    }

    basic_keeper<owner_category_tag, T> copy() &
    {
      return basic_keeper<owner_category_tag, T>(value_type{get()});
    }

    basic_keeper<owner_category_tag, std::remove_const_t<T>> mutable_copy() const &
    {
      using MutableT = std::remove_const_t<T>;
      return basic_keeper<owner_category_tag, MutableT>(MutableT{get()});
    }

    operator basic_keeper<reference_category_tag, const T>()
    {
      return basic_keeper<reference_category_tag, const T>{t};
    }

    template <typename Fn>
    decltype(auto) apply(Fn&& fn)
    {
      return std::invoke(fn, t);
    }
  };

  // maybe reference should work only with lvalue-refs and should not support moving at all
  // but it might not play well in generic code

  template <typename T>
  using owner = basic_keeper<owner_category_tag, T>;

  template <typename T>
  using reference_to = basic_keeper<reference_category_tag, T>;

  template <typename T>
  struct is_keeper : std::false_type {};

  template <typename Category, typename Value>
  struct is_keeper<basic_keeper<Category, Value>> : std::true_type {};

  template <typename K, typename T>
  struct is_keeper_for : std::false_type {};

  template <typename Category, typename Value>
  struct is_keeper_for<basic_keeper<Category, Value>, Value> : std::true_type {};

  static_assert(is_keeper_for<basic_keeper<owner_category_tag, int>, int>::value);
  static_assert(!is_keeper_for<basic_keeper<owner_category_tag, int>, double>::value);

  static_assert(is_keeper_for<basic_keeper<reference_category_tag, int>, int>::value);
  static_assert(!is_keeper_for<basic_keeper<reference_category_tag, int>, double>::value);

  namespace detail
  {
    template <typename T>
    struct keeper_value_type
    {
      using type = T;
    };

    template <typename Category, typename Value>
    struct keeper_value_type<basic_keeper<Category, Value>>
    {
      using type = Value;
    };

    template <typename T>
    using keeper_value_type_t = typename keeper_value_type<T>::type;

    /**
     * ownership_category is general, decides ownership category tag, based on T's value category.
     * It is so generic that it does not care keepers at all.
     * (better name?)
     */
    template <typename T>
    struct ownership_category : std::conditional<std::is_lvalue_reference_v<T>, reference_category_tag, owner_category_tag>
    {};

    template <typename T>
    using ownership_category_t = typename ownership_category<T>::type;

    static_assert(std::is_same_v<ownership_category_t<int>, owner_category_tag>);
    static_assert(std::is_same_v<ownership_category_t<int&&>, owner_category_tag>);
    static_assert(std::is_same_v<ownership_category_t<int&>, reference_category_tag>);

    // it does not care keepers
    static_assert(std::is_same_v<ownership_category_t<basic_keeper<owner_category_tag, int>>, owner_category_tag>);
    static_assert(std::is_same_v<ownership_category_t<basic_keeper<owner_category_tag, int>&>, reference_category_tag>);
    static_assert(std::is_same_v<ownership_category_t<basic_keeper<owner_category_tag, int>&&>, owner_category_tag>);

    /**
     * keeper_category
     */
    template <typename T>
    struct keeper_category : ownership_category<T> {};

    template <typename Category, typename Value>
    struct keeper_category<basic_keeper<Category, Value>>
    {
      using type = Category;
    };

    /**
     * keeper_category is not about deducing internal category, but to decide that you can own (move-from) the
     * object or just refer to. Maybe it should have a better name
     * */
    template <typename Category, typename Value>
    struct keeper_category<basic_keeper<Category, Value>&&>
    {
      using type = Category;
    };

    template <typename T>
    using keeper_category_t = typename keeper_category<T>::type;

    // for non-keepers the same as ownership_category
    static_assert(std::is_same_v<keeper_category_t<int>, owner_category_tag>);
    static_assert(std::is_same_v<keeper_category_t<int&&>, owner_category_tag>);
    static_assert(std::is_same_v<keeper_category_t<int&>, reference_category_tag>);

    // forwards keepers category
    static_assert(std::is_same_v<keeper_category_t<basic_keeper<owner_category_tag, int>>, owner_category_tag>);
    static_assert(std::is_same_v<keeper_category_t<basic_keeper<reference_category_tag, int>>, reference_category_tag>);

    // here it becomes weird
    static_assert(std::is_same_v<keeper_category_t<basic_keeper<owner_category_tag, int>>, owner_category_tag>);
    static_assert(std::is_same_v<keeper_category_t<basic_keeper<owner_category_tag, int>&>, reference_category_tag>); // reference, because it cannot be moved from it (without explicit move)
    static_assert(std::is_same_v<keeper_category_t<basic_keeper<owner_category_tag, int>&&>, owner_category_tag>);

    static_assert(std::is_same_v<keeper_category_t<basic_keeper<reference_category_tag, int>>, reference_category_tag>);
    static_assert(std::is_same_v<keeper_category_t<basic_keeper<reference_category_tag, int>&>, reference_category_tag>);
    static_assert(std::is_same_v<keeper_category_t<basic_keeper<reference_category_tag, int>&&>, reference_category_tag>);

    template <typename T>
    constexpr decltype(auto) get_keeper_value(T&& t) noexcept
    {
      if constexpr (is_keeper<std::remove_reference_t<T>>::value)
      {
        return std::forward<T>(t).get();
      }
      else
      {
        return std::forward<T>(t);
      }
    }
  }


  template <typename T>
  [[nodiscard]] constexpr decltype(auto) make_keeper(T&& t)
  {
    using ParameterCategory = detail::keeper_category_t<T>;
    using ValueType = detail::keeper_value_type_t<std::remove_reference_t<T>>;
    return basic_keeper<ParameterCategory, ValueType>{detail::get_keeper_value(std::forward<T>(t))};
  }

  // make owner -> copies from reference
  // make reference -> refers
  //
  /*
  owner<Person> o{Person{}};
  auto o = own(Person{});

  ref_to<Person> r{p};
  auto r = ref(p);
  */

}


#endif
