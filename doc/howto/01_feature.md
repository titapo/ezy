# Defining custom features

Compilable version of the following examples can be found [here](../../examples/howto/01_feature.cc).

## The most dumb feature

The basic skeleton of a feature is the following:

```cpp

struct answerable
{
  template <typename StrongType>
  struct impl
  {
    constexpr int answer() const
    {
      return 42;
    }
  };
};
```

Now break it apart:
* `struct answerable`: is the name of the feature, which has to have an `impl` member template
* `impl` is a nested struct (or type alias) which has exactly one type template argument `StrongType` which will be
  substituted of the concrete instantiated `ezy::strong_type<...>` type. More on this later.
* Within `impl` everything will be inherited for the strong type example.


```
using AnswerableInt = ezy::strong_type<int, struct my_tag, answerable>;

constexpr AnswerableInt ans{10};

 static_assert(ans.answer() == 42);
```

## Forwarding feature

Let's suppose that there is a `Door` class providing -- among others -- a method `.open()`.

```
struct Door
{
  enum class Status : bool
  {
    Open,
    Closed
  };

  Status status{Status::Closed};

  void open()
  {
    status = Status::Open;
  }

  void close()
  {
    status = Status::Closed;
  }

  Status getStatus() const
  {
    return status;
  }
};
```

 By wrapping it in an
`ezy::strong_type<Door, ...> strong_door` it could be called only as `strong_door.get().open()`, which is quite
cumbersome. It can be fixed the following way.

```cpp
struct openable
{
  template <typename T>
  struct impl
  {
    void open()
    {
      static_cast<T&>(*this).get().open();
    }
  };
};

using OpenableDoor = ezy::strong_type<Door, struct my_tag, openable>;
OpenableDoor d1;
d1.open(); // calls Door::open() under the hood
```

If one is not familiar with the CRTP (Curiously Recurring Template Pattern), might need some explanation of the
`static_cast`: the struct `openable::impl<T>` is a base class of `T` (which is `OpenableDoor`, also known as
`ezy::strong_type<Door, struct my_tag, openable>`), therefore `this.get().open()` cannot be resolved, because they are
defined in the derived class. Here we can use the `T` parameter and take advantage of the fact that `T` is a subclass of
`openable::impl<T>`, so `this` pointer can be downcasted to it.

The `static_cast` can be `const`-qualified if needed.


## Composing features

Now it would be quite easy to forward the `Door::close()` call as well:

```cpp
struct closable
{
  template <typename T>
  struct impl
  {
    void close()
    {
      static_cast<T&>(*this).get().close();
    }
  };
};
```


```cpp
using RegularDoor = ezy::strong_type<Door, struct my_tag, openable, closable>;
RegularDoor door;
door.open(); // calls Door::open() under the hood
door.close(); // calls Door::close() under the hood
```

So far, so good, except that that a door is always openable *and* closable, so there should be only one feature to cover
both within only one feature.

> In a concrete real-work case one should consider adding the `close()` method to `openable` feature, since it is
> tightly related to `open()`.

Two features can be composed in the following way:

```cpp
struct accessible
{
  template <typename T>
  struct impl : openable::impl<T>, closable::impl<T>
  {};
};
```

Considering inheritance as an *is-a* relationship this is quite self explaining. Now this new feature can be used in the
example above.

```cpp
using RegularDoor = ezy::strong_type<Door, struct my_tag, accessible>;
RegularDoor door;
```

## Internalization

One powerful aspect of features is taking an existing functionality and make it internal: if it was defined withing this
structure. If you are familiar with the idea of UFCS (Unified Function Call Syntax),  this will be a way to mimic this.

There is a `toggle` free function:

```cpp
void toggle(Door& door)
{
  if (door.getStatus() == Door::Status::Open)
    door.close();
  else
    door.open();
}
```

One can argue, this method should have been defined in `Door` as a member function, but this is not necessarily the case.
Anyway we can declare a feature to workaround this issue without touching the original `Door` interface:

```cpp
struct toggleable
{
  template <typename T>
  struct impl
  {
    void toggle()
    {
      ::toggle(static_cast<T&>(*this).get());
    }
  };
};
```

Please note the `::` qualification which is required to avoid `toggle` calling itself recursively.

```cpp
using ToggleableDoor = ezy::strong_type<Door, struct my_tag, toggleable>;
ToggleableDoor door;
door.toggle();
assert(Door::Status::Open == door.get().getStatus());
door.toggle();
assert(Door::Status::Closed == door.get().getStatus());
```

Now we can extend the previously declared `accessible` feature by adding `toggleable` as well. There is one more
opportunity we can do: `toggleable::impl<T>::toggle()` does not return anything, so we can do one of the following:

### Method chaining

```cpp
    auto& toggleable::impl<T>::toggle()
    {
      ::toggle(static_cast<T&>(*this).get());
      static_cast<T&>(*this);
    }
```

This practice allows method chaining, writing something like: `door.toggle().toggle();`

### Enchanted method

```cpp
    auto toggleable::impl<T>::toggle()
    {
      ::toggle(static_cast<T&>(*this).get());
      static_cast<T&>(*this).get().getStatus(); // returning the new status
    }
```

This can be useful, but please note that now the `toggleable` feature requires `getStatus()` method to be defined for
any other class. It is preferred to be conservative what features require from the interface they use.

## Extending features

By extending features we mean adding a new functionality to a class. Now we want a door which can be opened only by a
secret word.

```cpp
struct key_openable
{
  template <typename T>
  struct impl
  {
    void open(int key)
    {
      if (key == 42)
        static_cast<T&>(*this).get().open();
    }
  };
};
```

Now we can have some rules or guidance which should be followed:
* Before one would declare a member for holding the secret key. Avoid state in features: this is unfortunate to
   increase the size based of the underlying type's size. If that's the case consider defining a new type, containing
   the secret key and the Door as well. (See the example a bit later)
* If there is a parameter which is known in compile time you can add it as a template parameter (more on this later)
* Consider leading back *extension* to *internalization*:
  * define the new functionality as free function(s)
  * write a feature to internalize it/them


```cpp
void open_with_key(Door& door, int key)
{
  if (key == 42)
    door.open();
}

struct key_openable
{
  template <typename T>
  struct impl
  {
    void open(int key)
    {
      return ::open_with_key(static_cast<T&>(*this).get(), key);
    }
  };
};

void extend_with_secret()
{
  using SecretDoor = ezy::strong_type<Door, struct my_tag, key_openable>;
  SecretDoor door;
  door.open(10);
  assert(Door::Status::Closed == door.get().getStatus());
  door.open(42);
  assert(Door::Status::Open == door.get().getStatus());
}
```

Or having the secret as a member:

```cpp
struct SecretDoor2 : ezy::strong_type<Door, struct my_tag, closable>
{
  SecretDoor2(int secret_key)
    : secret(secret_key)
  {}

  void open(int key)
  {
    if (key == secret)
      this->get().open();
  }

  const int secret;
};

void extend_with_secret2()
{
  SecretDoor2 door{100};
  door.open(10);
  assert(Door::Status::Closed == door.get().getStatus());
  door.open(42);
  assert(Door::Status::Closed == door.get().getStatus());
  door.open(100);
  assert(Door::Status::Open == door.get().getStatus());
}
```

As you can see deriving from a strong type is possible. Although it reduces the cases of reusability of this
*extension*. `close()` function is inherited from the `closable` feature.

However when secret key is known in compile time it can be used as a template parameter of the feature:

```cpp
template <int Secret = 42>
struct static_key_openable
{
  template <typename T>
  struct impl
  {
    void open(int key)
    {
      if (key == Secret)
        static_cast<T&>(*this).get().open();
    }
  };
};


void extend_with_secret3()
{
  using SecretDoor3 = ezy::strong_type<Door, struct my_tag, static_key_openable<200>>;
  SecretDoor3 door;
  door.open(10);
  assert(Door::Status::Closed == door.get().getStatus());
  door.open(42);
  assert(Door::Status::Closed == door.get().getStatus());
  door.open(200);
  assert(Door::Status::Open == door.get().getStatus());
}
```

There is a lot of possible options, choose wisely based on specific needs.

