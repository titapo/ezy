#include <ezy/strong_type.h>
#include <cassert>

// simple feature
struct answerable
{
  template <typename T>
  struct impl
  {
    constexpr int answer() const
    {
      return 42;
    }
  };
};

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

struct accessible
{
  template <typename T>
  struct impl : openable::impl<T>, closable::impl<T>
  {};
};

void toggle(Door& door)
{
  if (door.getStatus() == Door::Status::Open)
    door.close();
  else
    door.open();
}

struct toggleable
{
  template <typename T>
  struct impl
  {
    T& toggle()
    {
      ::toggle(static_cast<T&>(*this).get());
      return static_cast<T&>(*this);
    }
  };
};


void internalization()
{
  using ToggleableDoor = ezy::strong_type<Door, struct my_tag, toggleable>;
  ToggleableDoor door;
  door.toggle();
  assert(Door::Status::Open == door.get().getStatus());
  door.toggle();
  assert(Door::Status::Closed == door.get().getStatus());
}

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

int main()
{
  using AnswerableInt = ezy::strong_type<int, struct my_tag, answerable>;
  constexpr AnswerableInt ans{10};
  static_assert(ans.answer() == 42);

  using OpenableDoor = ezy::strong_type<Door, struct my_tag, openable>;
  OpenableDoor d1;
  d1.open();
  assert(Door::Status::Open == d1.get().getStatus());

  using ClosableDoor = ezy::strong_type<Door, struct my_tag, closable>;
  ClosableDoor d2;
  d2.close();
  assert(Door::Status::Closed == d2.get().getStatus());

  //using RegularDoor = ezy::strong_type<Door, struct my_tag, openable, closable>;
  using RegularDoor = ezy::strong_type<Door, struct my_tag, accessible>;
  RegularDoor d3;
  d3.open();
  assert(Door::Status::Open == d3.get().getStatus());
  d3.close();
  assert(Door::Status::Closed == d3.get().getStatus());

  internalization();
  extend_with_secret();
  extend_with_secret2();
}
