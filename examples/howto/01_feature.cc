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

  Status status;

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
}
