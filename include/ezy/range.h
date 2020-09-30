#ifndef STASH_RANGE_H_INCLUDED
#define STASH_RANGE_H_INCLUDED

#include "experimental/tuple_algorithm.h"
#include "experimental/keeper.h"
#include "invoke.h"

#include <type_traits>
#include <utility>
#include <iterator>

#include <cstddef>
#include <tuple>
#include <limits>

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
    using type = decltype(std::declval<T>().begin());
  };

  template <typename T>
  using iterator_type_t = typename iterator_type<T>::type;

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
}
}

namespace ezy
{
namespace detail
{
  template <typename F>
  using IsFunction = typename std::enable_if<std::is_function<F>::value, F>;
}
}

namespace ezy
{
namespace detail
{
  template <typename orig_type>
  struct basic_iterator_adaptor
  {
    public:
      using difference_type = typename std::iterator_traits<orig_type>::difference_type;
      using value_type = typename std::iterator_traits<orig_type>::value_type;
      using pointer = typename std::iterator_traits<orig_type>::pointer;
      using reference = typename std::iterator_traits<orig_type>::reference;
      using iterator_category = typename std::iterator_traits<orig_type>::iterator_category;

      constexpr explicit basic_iterator_adaptor(const orig_type& original)
          : orig(original)
      {}

      inline constexpr basic_iterator_adaptor& operator++()
      {
          ++orig;
          return *this;
      }

      inline constexpr decltype(auto) operator*()
      {
        return *orig;
      }

      inline constexpr bool operator==(const basic_iterator_adaptor& rhs) const
      { return orig == rhs.orig; }

      inline constexpr bool operator!=(const basic_iterator_adaptor& rhs) const
      { return orig != rhs.orig; }

      inline constexpr auto operator+=(difference_type diff)
      { return orig += diff; }

      inline constexpr auto operator-(const orig_type& rhs)
      { return orig - rhs; }

      inline constexpr auto operator-(const basic_iterator_adaptor& rhs)
      { return orig - rhs.orig; }

      orig_type orig;
  };

  // tag for mark end iterator - experimental
  struct end_marker_t
  {};

  /**
   * iterator tracker is a collection of iterators to Ranges.
   *
   * It's convenient for algorithms which operates multiple range at the same time.
   */
  template <typename... Iters>
  struct iterator_tracker
  {
    public:
      constexpr static auto size = sizeof...(Iters);

      template <typename... Iterators>
      constexpr iterator_tracker(Iterators&&... p_iters)
        : iters{std::forward<Iterators>(p_iters)...}
      {}

      template <typename... Ranges>
      constexpr static iterator_tracker begin_from_ranges(Ranges&... ranges)
      {
        using std::begin;
        return iterator_tracker(begin(ranges)...);
      }

      template <typename... Ranges>
      constexpr static iterator_tracker end_from_ranges(Ranges&... ranges)
      {
        using std::end;
        return iterator_tracker(end(ranges)...);
      }

      template <unsigned N>
      constexpr decltype(auto) get() noexcept
      {
        return std::get<N>(iters);
      }

      template <unsigned N>
      constexpr decltype(auto) get() const noexcept
      {
        return std::get<N>(iters);
      }

      template <unsigned N, typename IterT>
      void set_to(IterT&& it)
      {
        get<N>() = std::forward<IterT>(it);
      }

      template <unsigned N>
      decltype(auto) next()
      {
        return ++std::get<N>(iters);
      }

      constexpr void next_all()
      {
        ezy::experimental::static_for_each(iters, [](auto& it){ ++it; });
      }

    private:
      std::tuple<Iters...> iters;
  };

  template <typename... Ranges>
  using iterator_tracker_for = iterator_tracker<iterator_type_t<Ranges>...>;

  /**
   * range_tracker is a more heavyweight version of iterator tracker.
   *
   * Alongside with an iterator it stores a reference to the range itself. So it can access the range
   * properties, but it may provide unecessary lazyness.
   *
   * Idea: this should contain {it, end} iterator pairs instead.
   *
   * Since a range tracker itself is able to notice when the iterator reaches the end, it should not be used
   * in an end iterator. But swiching to iterator+sentinel begin/end implementation would cause
   * interoperability problems eg. with standard algorithms, and container constructors whose takes iterator
   * pairs.
   */

  template <typename... Ranges>
  struct range_tracker
  {
    public:
      constexpr static auto size = sizeof...(Ranges);

      range_tracker(Ranges&... ranges)
        : ranges(ranges...)
        , current(ranges.begin()...)
      {}

      range_tracker(Ranges&... ranges, end_marker_t&&)
        : ranges(ranges...)
        , current(std::end(ranges)...)
      {}

      range_tracker(const range_tracker& rhs)
        : ranges(rhs.ranges)
        , current(rhs.current)
      {}

      template <typename... OtherRanges>
      range_tracker(const range_tracker<OtherRanges...>& rhs)
        : ranges(rhs.ranges)
        , current(rhs.current)
      {
      }

      range_tracker& operator=(const range_tracker& rhs)
      {
        ezy::experimental::static_for<size>([this, rhs](auto i)
            {
              std::get<i>(ranges) = std::get<i>(rhs.ranges);
            });
        current = rhs.current;
        return *this;
      }

      template <unsigned N>
      auto get()
      {
        using it_type = decltype(std::get<N>(current));
        using end_it_type = decltype(std::get<N>(ranges).get().end());
        return std::pair<it_type, const end_it_type>(std::get<N>(current), std::get<N>(ranges).get().end());
      }

      template <unsigned N>
      auto get() const
      {
        using it_type = decltype(std::get<N>(current));
        using end_it_type = decltype(std::get<N>(ranges).get().end());
        return std::pair<it_type, const end_it_type>(std::get<N>(current), std::get<N>(ranges).get().end());
      }

      template <unsigned N, typename ItT>
      void set_to(ItT it)
      {
        std::get<N>(current) = it;
      }

      template <unsigned N>
      void set_to_begin()
      {
        set_to<N>(std::begin(std::get<N>(ranges).get()));
      }

      template <unsigned N>
      void set_to_end()
      {
        set_to<N>(std::get<N>(ranges).get().end());
      }

      template <unsigned N>
      decltype(auto) next()
      {
        return ++std::get<N>(current);
      }

      constexpr void next_all()
      {
        ezy::experimental::static_for_each(current, [](auto& it){ ++it; });
      }

      template <unsigned N>
      bool has_next() const
      {
        return std::get<N>(current) != std::get<N>(ranges).get().end();
      }

    private:
    public:
      template <typename RangeType>
      using const_it_type = ezy::detail::iterator_type_t<RangeType>; //typename RangeType::const_iterator;

      std::tuple<std::reference_wrapper<Ranges>...> ranges;
      std::tuple<const_it_type<Ranges>...> current;
  };


  template <typename orig_type,
           typename converter_type
           // , typename = IsFunction<converter_type>
           >
  struct iterator_adaptor : basic_iterator_adaptor<orig_type>
  {
    public:
      using base = basic_iterator_adaptor<orig_type>;
      using value_type = decltype(*std::declval<orig_type>());
      using result_type = decltype(std::declval<converter_type>()(std::declval<value_type>()));

      // constructor
      using base::basic_iterator_adaptor;

      /*
      iterator_adaptor(orig_type original, converter_type&& c)
        : base(original)
        , converter(std::move(c))
      {}
      */

      constexpr iterator_adaptor(const orig_type& original, const converter_type& c)
        : base(original)
        , converter(c)
      {}

      inline constexpr iterator_adaptor operator+(int increment) const
      { return iterator_adaptor(base::orig + increment); }

      inline constexpr iterator_adaptor& operator++()
      {
        ++base::orig;
        return *this;
      }

      constexpr result_type operator*()
      {
        return converter(*(base::orig));
      }

    private:
      converter_type converter;
  };

  /**
   * iterator_filter
   */
  template <typename orig_type,
            typename predicate_type
           >
  struct iterator_filter : basic_iterator_adaptor<orig_type>
  {
    public:
      using base = basic_iterator_adaptor<orig_type>;
      using value_type = decltype(*std::declval<orig_type>());
      using difference_type = typename base::difference_type;
      using iterator_category = std::input_iterator_tag; // forward_iterator_tag?

      // constructor
      using base::basic_iterator_adaptor;


      /*
      iterator_filter(orig_type original, predicate_type&& p, const orig_type& end)
        : base(original)
        , converter(std::move(c))
      {}
      */

      iterator_filter(orig_type original, const predicate_type& p, const orig_type& end)
        : base(original)
        , predicate(p)
        , end_iterator(end)
      {
        if (base::orig != end_iterator && !predicate(*base::orig))
          operator++();
      }

      inline iterator_filter& operator++()
      {
        ++base::orig;
        for (; base::orig != end_iterator; ++base::orig)
          if (predicate(*base::orig))
            return *this;

        return *this;
      }

      value_type operator*()
      {
        return *(base::orig);
      }

    private:
      predicate_type predicate;
      const orig_type end_iterator;
  };

  template <typename first_range_type, typename second_range_type>
  struct iterator_concatenator
  {
    public:
      using orig_type = typename first_range_type::const_iterator;
      using value_type = decltype(*std::declval<orig_type>());
      using pointer = std::add_pointer_t<value_type>;
      using reference = std::add_lvalue_reference_t<value_type>;
      using difference_type = typename orig_type::difference_type;
      using iterator_category = std::input_iterator_tag; // forward_iterator_tag?

      // constructor

      iterator_concatenator(const first_range_type& fr, const second_range_type& sr)
        : tracker(fr, sr)
      {}

      iterator_concatenator(const first_range_type& fr, const second_range_type& sr, end_marker_t&&)
        : tracker(fr, sr, end_marker_t{})
      {}

      inline iterator_concatenator& operator++()
      {
        {
          auto tracked = tracking_info<0>();
          if (tracked.first != tracked.second)
          {
            ++tracked.first;
            return *this;
          }
        }

        {
          auto tracked = tracking_info<1>();
          if (tracked.first != tracked.second)
          {
            ++tracked.first;
            return *this;
          }
        }

        return *this;
      }

      auto operator*()
      {
        auto tracked = tracking_info<0>();
        if (tracked.first != tracked.second)
        {
          return *tracked.first;
        }

        return *tracking_info<1>().first;
      }

      bool operator==(const iterator_concatenator& rhs) const
      {
        {
          const auto tracked = tracking_info<0>();
          if (tracked.first != tracked.second)
          {
            const auto r_tracked = rhs.tracking_info<0>();
            if (r_tracked.first != r_tracked.second)
              return (r_tracked.first == tracked.first);
            else
              return false;
          }
        }

        {
          const auto tracked = tracking_info<1>();
          if (tracked.first != tracked.second)
          {
            const auto r_tracked = rhs.tracking_info<1>();
            if (r_tracked.first != r_tracked.second)
              return (r_tracked.first == tracked.first);
            else
              return false;
          }
        }

        // this ended
        return !rhs.tracker.template has_next<1>();
      }

      bool operator!=(const iterator_concatenator& rhs) const
      {
        return !(*this == rhs);
      }

      template <unsigned N>
      auto tracking_info()
      {
        return tracker.template get<N>();
      }

      template <unsigned N>
      auto tracking_info() const
      {
        return tracker.template get<N>();
      }

    private:
      range_tracker<first_range_type, second_range_type> tracker;
  };

  template <typename range_type>
  struct iterator_flattener
  {
    public:
      using orig_iterator = typename range_type::const_iterator;
      using inner_iterator = typename orig_iterator::value_type::const_iterator;

      using value_type = typename inner_iterator::value_type;
      using difference_type = typename inner_iterator::difference_type;
      using pointer = typename inner_iterator::pointer;
      using reference = typename inner_iterator::reference;
      using iterator_category = std::forward_iterator_tag;

      iterator_flattener(const range_type& range)
        : tracker(range)
      {
        if (tracker.template has_next<0>())
          inner = outer()->begin();
      }

      iterator_flattener(const range_type& range, end_marker_t&&)
        : tracker(range, end_marker_t{})
      {
        if (tracker.template has_next<0>())
          inner = outer()->end();
      }

      iterator_flattener& operator++()
      {
        ++inner;
        auto outer_tracked = tracker.template get<0>();
        while (outer_tracked.first != outer_tracked.second && inner == outer_tracked.first->end()) // end of current subrange
        {
          ++outer_tracked.first;
          if (outer_tracked.first != outer_tracked.second) // not finished
          {
            inner = outer()->begin();
          }
        }

        return *this;
      }

      const value_type& operator*() const
      {
        return *inner;
      }

      friend bool operator==(const iterator_flattener& lhs, const iterator_flattener& rhs)
      {
        const auto& lhs_tracker = lhs.tracker.template get<0>();
        const auto& rhs_tracker = rhs.tracker.template get<0>();
        if (lhs_tracker.first != rhs_tracker.first)
          return false;

        if (lhs_tracker.first != lhs_tracker.second
            && rhs_tracker.first != rhs_tracker.second
            && lhs.inner != rhs.inner)
          return false;

        return true;
      }

      bool operator!=(const iterator_flattener& rhs) const
      {
        return !(*this == rhs);
      }

      orig_iterator& outer()
      {
        return tracker.template get<0>().first;
      }

    private:
        range_tracker<range_type> tracker;
        inner_iterator inner;
  };

  template <typename Zipper, typename... Ranges>
  struct iterator_zipper
  {
    public:
      using difference_type = size_t; // TODO what should it be?
      using value_type = std::remove_reference_t<decltype(
          ezy::invoke(std::declval<Zipper>(), (*std::begin(std::declval<Ranges>()))...)
          )>;
      using pointer = std::add_pointer_t<value_type>;
      using reference = std::add_lvalue_reference_t<value_type>;
      using iterator_category = std::input_iterator_tag; // forward_iterator_tag?

      using DEBUG = std::tuple<Ranges...>;
      using DEBUG2 = std::tuple<iterator_type_t<Ranges>...>;
      using tracker_type = iterator_tracker_for<Ranges...>;

      static constexpr auto cardinality = sizeof...(Ranges);

      constexpr iterator_zipper(Zipper zipper, Ranges&... rs)
        : storage(zipper, tracker_type::begin_from_ranges(rs...))
      {}

      constexpr iterator_zipper(Zipper zipper, Ranges&... rs, end_marker_t&&)
        : storage(zipper, tracker_type::end_from_ranges(rs...))
      {}

    private:
      static constexpr size_t zipper_index{0};
      static constexpr size_t tracker_index{1};

      constexpr decltype(auto) tracker()
      {
        return std::get<tracker_index>(storage);
      }

      constexpr decltype(auto) tracker() const
      {
        return std::get<tracker_index>(storage);
      }

      constexpr decltype(auto) zipper()
      {
        return std::get<zipper_index>(storage);
      }

      template <size_t... Is>
      constexpr auto deref_helper(std::index_sequence<Is...>)
      {
        return zipper()(
            (*(tracker().template get<Is>()))...
            );
      }

      template <size_t... Is>
      constexpr static bool has_next_helper(
          const tracker_type& tracker,
          const tracker_type& rhs,
          std::index_sequence<Is...>
        )
      {
        return ((tracker.template get<Is>() != rhs.template get<Is>()) && ...);
      }

    public:
      constexpr auto operator*()
      {
        return deref_helper(std::make_index_sequence<cardinality>());
      }

      constexpr iterator_zipper& operator++()
      {
        tracker().next_all();
        return *this;
      }

      constexpr bool operator!=(const iterator_zipper& rhs) const
      {
        return has_next_helper(tracker(), rhs.tracker(), std::make_index_sequence<cardinality>());
      }

      constexpr bool operator==(const iterator_zipper& rhs) const
      {
        return !(*this != rhs);
      }

    private:
      std::tuple<Zipper, tracker_type> storage;
  };

/*
  template <typename orig_type>
  struct iterator_group_adaptor : basic_iterator_adaptor<orig_type>
  {
      using base = basic_iterator_adaptor<orig_type>;
      using orig_value_type = decltype(*std::declval<orig_type>());
      using value_type = range_view_slice<orig_value_type>;
      using difference_type = typename base::difference_type;
      using iterator_category = std::input_iterator_tag; // forward_iterator_tag?

      iterator_group_adaptor(const orig_type& original, const orig_type::difference_type gs, const orig_type& end)
        : base(original)
        , group_size(gs)
        , current_position(0)
        , end_iterator(end)
    {
    }

    inline iterator_group_adaptor& operator++()
    {
      const auto& target_position = (current_position + group_size)
      for (; (current_position < target_position); ++base::orig)
        if (base::orig == end_iterator)
          return *this;

      return *this;
    }

    value_type operator*()
    {
      return range_view_slice<orig_value_type>(base::orig);
    }


    private:
      const orig_type::difference_type group_size;
      orig_type::difference_type current_position;
      const orig_type end_iterator;
  }
  */

  template <typename RangeType>
  struct take_iterator
  {
    public:
      using difference_type = typename RangeType::const_iterator::difference_type;
      using value_type = typename RangeType::const_iterator::value_type;
      using pointer = typename RangeType::const_iterator::pointer;
      using reference = typename RangeType::const_iterator::reference;
      using iterator_category = std::forward_iterator_tag; // ??

      explicit take_iterator(const RangeType& range, typename RangeType::size_type n)
        : tracker(range)
        , n(n)
      {}

      explicit take_iterator(const RangeType& range, end_marker_t)
        : tracker(range)
        , n(0)
      {}

      take_iterator& operator=(const take_iterator& rhs)
      {
        tracker = rhs.tracker;
        n = rhs.n;
        return *this;
      }

      /*
      take_iterator& operator=(take_iterator&& rhs) //noexcept
      {
        tracker = std::move(rhs.tracker);
        n = rhs.n;
        return *this;
      }
      */

      inline take_iterator& operator++()
      {
        tracker.template next<0>();
        --n;
        return *this;
      }

      decltype(auto) operator*()
      {
        return *(tracker.template get<0>().first);
      }

      bool operator!=(const take_iterator&) const
      {
        return (n != 0 && tracker.template has_next<0>());
      }

      bool operator==(const take_iterator& rhs) const
      {
        return !(*this != rhs);
      }

    private:
      range_tracker<RangeType> tracker;
      typename RangeType::size_type n;
  };

  template <typename RangeType, typename Predicate>
  struct take_while_iterator
  {
    public:
      using difference_type = typename RangeType::const_iterator::difference_type;
      using value_type = typename RangeType::const_iterator::value_type;
      using pointer = typename RangeType::const_iterator::pointer;
      using reference = typename RangeType::const_iterator::reference;
      using iterator_category = std::forward_iterator_tag; // ??

      explicit take_while_iterator(const RangeType& range, Predicate p)
        : tracker(range)
        , predicate(std::move(p))
      {
        const auto tracked = tracker.template get<0>();
        const auto &it = tracked.first;
        const auto &end = tracked.second;
        if (it != end && !predicate(*it))
          tracker.template set_to<0>(end);
      }

      explicit take_while_iterator(const RangeType& range, Predicate p, end_marker_t)
        : tracker(range)
        , predicate(std::move(p))
      {
        const auto tracked = tracker.template get<0>();
        tracker.template set_to<0>(tracked.second);
      }

      inline take_while_iterator& operator++()
      {
        tracker.template next<0>();

        const auto tracked = tracker.template get<0>();
        if (tracked.first == tracked.second)
          return *this;

        if (!predicate(*tracked.first))
          tracker.template set_to<0>(tracked.second);

        return *this;
      }

      decltype(auto) operator*()
      {
        return *(tracker.template get<0>().first);
      }

      bool operator!=(const take_while_iterator&) const
      {
        // TODO fix it
        const auto tracked = tracker.template get<0>();
        return tracked.first != tracked.second;
      }

      bool operator==(const take_while_iterator& rhs) const
      {
        return !(*this != rhs);
      }

    private:
      range_tracker<RangeType> tracker;
      Predicate predicate;
  };

  template <typename Range>
  struct drop_iterator
  {
    using _iter_traits = std::iterator_traits<iterator_type_t<Range>>;
    using difference_type = typename _iter_traits::difference_type;
    using value_type = typename _iter_traits::value_type;
    using pointer = typename _iter_traits::pointer;
    using reference = typename _iter_traits::reference;
    using iterator_category = std::forward_iterator_tag; // TODO

    constexpr explicit drop_iterator(Range& range, typename Range::size_type n)
      : tracker(range)
    {
      while (tracker.template has_next<0>() && n > 0)
      {
        tracker.template next<0>();
        --n;
      }
    }

    constexpr explicit drop_iterator(Range& range, end_marker_t)
      : tracker(range, end_marker_t{})
    {
    }

    template <typename OtherRange, typename = std::enable_if_t<std::is_convertible<OtherRange&, Range&>::value>>
    constexpr drop_iterator(const drop_iterator<OtherRange>& other)
      : tracker(other.tracker)
    {
    }

    constexpr inline drop_iterator& operator++()
    {
      tracker.template next<0>();
      return *this;
    }

    constexpr decltype(auto) operator*()
    {
      return *(tracker.template get<0>().first);
    }

    constexpr bool operator!=(const drop_iterator&) const
    {
      return (tracker.template has_next<0>());
    }

    constexpr bool operator==(const drop_iterator& rhs) const
    {
      return !(*this != rhs);
    }

    range_tracker<Range> tracker;
  };

  template <typename Range>
  struct step_by_iterator
  {
    using _iter_traits = std::iterator_traits<iterator_type_t<Range>>;
    using difference_type = typename _iter_traits::difference_type;
    using value_type = typename _iter_traits::value_type;
    using pointer = typename _iter_traits::pointer;
    using reference = typename _iter_traits::reference;
    using iterator_category = std::forward_iterator_tag; // TODO

    constexpr explicit step_by_iterator(Range& range, typename Range::size_type n)
      : tracker(range)
      , n(n)
    {}

    constexpr explicit step_by_iterator(Range& range, end_marker_t)
      : tracker(range, end_marker_t{})
    {}

    template <typename OtherRange, typename = std::enable_if_t<std::is_convertible<OtherRange&, Range&>::value>>
    constexpr explicit step_by_iterator(const step_by_iterator<OtherRange>& other)
      : tracker(other.tracker)
      , n(other.n)
    {}

    constexpr step_by_iterator& operator++()
    {
      typename Range::size_type step = 0;
      while (step++ < n && tracker.template has_next<0>())
      {
        tracker.template next<0>();
      }
      return *this;
    }

    constexpr decltype(auto) operator*()
    {
      return *(tracker.template get<0>().first);
    }

    constexpr bool operator!=(const step_by_iterator& rhs) const
    {
      return tracker.template get<0>() != rhs.tracker.template get<0>();
    }

    constexpr bool operator==(const step_by_iterator& rhs) const
    {
      return !(*this != rhs);
    }

    range_tracker<Range> tracker;
    const typename Range::size_type n{1};
  };

  /**
   * basic_range_view
   */
  template <typename Range>
  struct basic_range_view
  {
    using const_iterator = basic_iterator_adaptor<typename Range::const_iterator>;

    constexpr basic_range_view(const Range& orig)
      : orig_range(orig)
    {}

    const Range& orig_range;
  };

  /**
   * range_view
   */
  template <typename Keeper, typename Transformation>
  struct range_view
  {
    using Range = ezy::experimental::keeper_value_type_t<Keeper>;
    using _orig_const_iterator = decltype(std::cbegin(std::declval<Range>()));
    using const_iterator = iterator_adaptor<_orig_const_iterator, Transformation>;
    using size_type = typename Range::size_type;

    constexpr const_iterator begin() const
    { return const_iterator(std::cbegin(orig_range.get()), transformation); }

    constexpr const_iterator end() const
    { return const_iterator(std::cend(orig_range.get()), transformation); }

    Keeper orig_range;
    Transformation transformation;
  };

  /**
   * range_view_filter
   */
  template <typename Keeper, typename FilterPredicate>
  struct range_view_filter
  {
    using Range = ezy::experimental::keeper_value_type_t<Keeper>;
    using const_iterator = iterator_filter<typename Range::const_iterator, FilterPredicate>;
    using size_type = typename Range::size_type;

    range_view_filter(Keeper&& keeper, FilterPredicate pred)
      : orig_range(std::move(keeper))
      , predicate(pred)
    {}

    const_iterator begin() const
    { return const_iterator(orig_range.get().begin(), predicate, orig_range.get().end()); }

    const_iterator end() const
    { return const_iterator(orig_range.get().end(), predicate, orig_range.get().end()); }

    private:
      Keeper orig_range;
      FilterPredicate predicate;
  };

  /**
   * range_view_slice
   */
  template <typename Keeper>
  struct range_view_slice
  {
    public:
      using Range = ezy::experimental::keeper_value_type_t<Keeper>;
      using const_iterator = typename Range::const_iterator;
      using difference_type = typename const_iterator::difference_type;
      using size_type = typename Range::difference_type;

      range_view_slice(Keeper&& orig, difference_type f, difference_type u)
        : orig_range(std::move(orig))
        , from(f)
        , until(u)
      {
        if (from > until)
          throw std::logic_error("logic error"); // programming error
      }

      const_iterator begin() const
      { return const_iterator(std::next(orig_range.get().begin(), bounded(from))); }

      const_iterator end() const
      { return const_iterator(std::next(orig_range.get().begin(), bounded(until))); }

    private:
      difference_type get_range_size() const
      {
        return std::distance(orig_range.get().begin(), orig_range.get().end());
      }

      difference_type bounded(difference_type difference) const
      {
        return std::min(get_range_size(), difference);
      }

      Keeper orig_range;
      const difference_type from;
      const difference_type until;
  };

  template <typename Range>
  struct group_range_view : basic_range_view<Range>
  {
    using base = basic_range_view<Range>;
    using const_iterator = typename Range::const_iterator;
    using difference_type = typename const_iterator::difference_type;

    group_range_view(const Range& orig, difference_type size)
      : base(orig)
      , group_size(size)
    {
    }

    const difference_type group_size;
  };

  // TODO generalize to N ranges (N > 0)
  template <typename Keeper1, typename Keeper2>
  struct concatenated_range_view
  {
    public:
      using const_iterator = iterator_concatenator<
        const ezy::experimental::keeper_value_type_t<Keeper1>,
        const ezy::experimental::keeper_value_type_t<Keeper2>
      >;
      using difference_type = typename const_iterator::difference_type;
      using size_type = typename ezy::experimental::keeper_value_type_t<Keeper1>::size_type;

      const_iterator begin() const
      { return const_iterator(range1.get(), range2.get()); }

      const_iterator end() const
      { return const_iterator(range1.get(), range2.get(), end_marker_t{}); }

    public:
    //private:
      Keeper1 range1;
      Keeper2 range2;
  };

  template <typename Zipper, typename... Keepers>
  struct zip_range_view
  {
    public:
      using KeepersTuple = std::tuple<Keepers...>;

      using iterator = iterator_zipper<Zipper, ezy::experimental::keeper_value_type_t<Keepers>...>;
      using const_iterator = iterator_zipper<Zipper, const ezy::experimental::keeper_value_type_t<Keepers>...>;
      using difference_type = typename const_iterator::difference_type;
      using value_type = typename const_iterator::value_type;
      using size_type = size_t;

      template <typename UZipper> // universal
      constexpr zip_range_view(UZipper&& zipper, Keepers&&... keepers)
        : zipper(std::forward<UZipper>(zipper))
        , keepers{std::move(keepers)...}
      {}

      template <size_t... Is>
      constexpr static const_iterator get_begin_helper(
          Zipper zipper,
          const KeepersTuple& keepers,
          std::index_sequence<Is...>
        )
      {
        return const_iterator(zipper, std::get<Is>(keepers).get()...);
      }

      template <size_t... Is>
      constexpr static iterator get_begin_helper(
          Zipper zipper,
          KeepersTuple& keepers,
          std::index_sequence<Is...>
        )
      {
        return iterator(zipper, std::get<Is>(keepers).get()...);
      }

      constexpr static const_iterator get_begin(Zipper zipper, const KeepersTuple& keepers)
      {
        return get_begin_helper(zipper, keepers, std::make_index_sequence<std::tuple_size<KeepersTuple>::value>());
      }

      constexpr static iterator get_begin(Zipper zipper, KeepersTuple& keepers)
      {
        return get_begin_helper(zipper, keepers, std::make_index_sequence<std::tuple_size<KeepersTuple>::value>());
      }

      template <size_t... Is>
      constexpr static const_iterator get_end_helper(
          Zipper zipper,
          const KeepersTuple& keepers,
          std::index_sequence<Is...>
        )
      {
        return const_iterator(zipper, std::get<Is>(keepers).get()..., end_marker_t{});
      }

      template <size_t... Is>
      constexpr static iterator get_end_helper(
          Zipper zipper,
          KeepersTuple& keepers,
          std::index_sequence<Is...>
        )
      {
        return iterator(zipper, std::get<Is>(keepers).get()..., end_marker_t{});
      }

      constexpr static const_iterator get_end(Zipper zipper, const KeepersTuple& keepers)
      {
        return get_end_helper(zipper, keepers, std::make_index_sequence<std::tuple_size<KeepersTuple>::value>());
      }

      constexpr static iterator get_end(Zipper zipper, KeepersTuple& keepers)
      {
        return get_end_helper(zipper, keepers, std::make_index_sequence<std::tuple_size<KeepersTuple>::value>());
      }

      constexpr const_iterator begin() const
      { return get_begin(zipper, keepers); }

      constexpr const_iterator end() const
      { return get_end(zipper, keepers); }

      constexpr iterator begin()
      { return get_begin(zipper, keepers); }

      constexpr iterator end()
      { return get_end(zipper, keepers); }

    public:
    //private:
      Zipper zipper;
      KeepersTuple keepers;
  };

  template <typename Keeper>
  struct flattened_range_view
  {
    public:

      using Range = ezy::experimental::keeper_value_type_t<Keeper>;
      using const_iterator = iterator_flattener<const Range>;

      using value_type = typename const_iterator::value_type;
      using pointer = typename const_iterator::pointer;
      using reference = typename const_iterator::reference;
      using difference_type = typename const_iterator::difference_type;
      using size_type = typename Range::size_type;

      const_iterator begin() const
      {
        return const_iterator(range.get());
      }

      const_iterator end() const
      {
        return const_iterator(range.get(), end_marker_t{});
      }

    //private:
      Keeper range;
  };

  template <typename Keeper>
  struct take_n_range_view
  {
    public:
      using Range = ezy::experimental::keeper_value_type_t<Keeper>;

      using const_iterator = take_iterator<const Range>;
      using size_type = typename Range::size_type;

      const_iterator begin() const
      {
        return const_iterator(range.get(), n);
      }

      const_iterator end() const
      {
        return const_iterator(range.get(), end_marker_t{});
      }

      Keeper range;
      size_type n;
  };

  template <typename Keeper>
  struct drop_range_view
  {
    using Range = ezy::experimental::keeper_value_type_t<Keeper>;
    using const_iterator = drop_iterator<const Range>;
    using iterator = drop_iterator<Range>;
    using size_type = typename Range::size_type;

    constexpr const_iterator begin() const
    {
      return const_iterator(range.get(), n);
    }

    constexpr const_iterator end() const
    {
      return const_iterator(range.get(), end_marker_t{});
    }

    constexpr iterator begin()
    {
      return iterator(range.get(), n);
    }

    constexpr iterator end()
    {
      return iterator(range.get(), end_marker_t{});
    }

    Keeper range;
    const size_type n;
  };

  template <typename Keeper>
  struct step_by_range_view
  {
    using Range = ezy::experimental::keeper_value_type_t<Keeper>;
    using const_iterator = step_by_iterator<const Range>;
    using iterator = step_by_iterator<Range>;
    using size_type = typename Range::size_type;

    constexpr const_iterator begin() const
    {
      return const_iterator(keeper.get(), n);
    }

    constexpr const_iterator end() const
    {
      return const_iterator(keeper.get(), end_marker_t{});
    }

    constexpr iterator begin()
    {
      return iterator(keeper.get(), n);
    }

    constexpr iterator end()
    {
      return iterator(keeper.get(), end_marker_t{});
    }

    Keeper keeper;
    size_type n{1};
  };

  template <typename Keeper, typename Predicate>
  struct take_while_range_view
  {
    public:
      using Range = ezy::experimental::keeper_value_type_t<Keeper>;
      using const_iterator = take_while_iterator<const Range, Predicate>;
      using size_type = typename Range::size_type;

      const_iterator begin() const
      {
        return const_iterator(range.get(), pred);
      }

      const_iterator end() const
      {
        return const_iterator(range.get(), pred, end_marker_t{});
      }

      Keeper range;
      Predicate pred;
  };

  template <typename T, typename Operation>
  struct iterate_iterator
  {
    using difference_type = T;
    using value_type = T;
    using reference = std::add_lvalue_reference_t<T>;
    using pointer = std::add_pointer_t<T>;
    using iterator_category = std::forward_iterator_tag;

    constexpr explicit iterate_iterator(T init, Operation op)
      : storage(init, op)
    {}

    constexpr reference operator*() noexcept
    {
      return std::get<0>(storage);
    }

    constexpr bool operator!=(const iterate_iterator&) const noexcept
    {
      return true;
    }
    constexpr bool operator==(const iterate_iterator& rhs) const noexcept
    {
      return !(*this != rhs);
    }

    constexpr iterate_iterator& operator++()
    {
      std::get<0>(storage) = ezy::invoke(std::get<1>(storage), std::get<0>(storage));
      return *this;
    }

    std::tuple<T, Operation> storage;
  };

  template <typename T, typename Operation>
  struct iterate_view
  {
    using const_iterator = iterate_iterator<T, Operation>;

    using size_type = size_t;

    constexpr const_iterator begin() const
    {
      return const_iterator(init, op);
    }

    constexpr const_iterator end() const
    {
      return const_iterator(std::numeric_limits<T>::max(), op);
    }

    T init{};
    Operation op;
  };

  template <typename Range>
  struct cycle_iterator
  {
    using orig_traits = std::iterator_traits<iterator_type_t<Range>>;
    using difference_type = typename orig_traits::difference_type;
    using value_type = typename orig_traits::value_type;
    using pointer = typename orig_traits::pointer;
    using reference = typename orig_traits::reference;

    decltype(auto) operator*()
    {
      return *(tracker.template get<0>().first);
    }

    cycle_iterator& operator++()
    {
      tracker.template next<0>();
      if (!tracker.template has_next<0>())
      {
        tracker.template set_to_begin<0>();
      }
      return *this;
    }

    bool operator!=(const cycle_iterator&) const
    {
      return true;
    }

    range_tracker<Range> tracker;
  };

  template <typename Keeper>
  struct cycle_view
  {
    using Range = ezy::experimental::keeper_value_type_t<Keeper>;
    using iterator = cycle_iterator<Range>;
    using const_iterator = cycle_iterator<const Range>;

    using size_type = typename Range::size_type;

    /*
    iterator begin()
    { return iterator{range.get()}; }

    iterator end()
    { return iterator{range.get()}; }
    */

    const_iterator begin() const
    { return const_iterator{range.get()}; }

    const_iterator end() const
    { return const_iterator{range.get()}; }

    Keeper range;
  };

  template <typename T>
  struct repeat_iterator
  {
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::forward_iterator_tag; // random_access

    decltype(auto) operator*()
    {
      return t;
    }

    decltype(auto) operator*() const
    {
      return t;
    }

    repeat_iterator& operator++()
    {
      return *this;
    }

    bool operator!=(const repeat_iterator&) const
    {
      return true;
    }

    T t;
  };

  template <typename T>
  struct repeat_view
  {
    using iterator = repeat_iterator<T>;
    using const_iterator = repeat_iterator<T>;

    using size_type = size_t;


    const_iterator begin() const
    { return const_iterator{t}; }

    const_iterator end() const
    { return const_iterator{t}; }

    T t; // as keeper?
  };
}
}

// TODO these should be in another header
namespace ezy
{
  template <size_t N>
  struct pick_nth_t
  {
    template <typename ValueType>
    decltype(auto) operator()(const ValueType& value) const
    { return std::get<N>(value); }
  };

  template <size_t N>
  constexpr pick_nth_t<N> pick_nth{};

  constexpr pick_nth_t<0> pick_first{};
  constexpr pick_nth_t<1> pick_second{};

  template <typename T>
  struct pick_type_t
  {
    template <typename ValueType>
    decltype(auto) operator()(const ValueType& value) const
    { return std::get<T>(value); }
  };

  template <typename T>
  constexpr pick_type_t<T> pick_type{};

}

#endif
