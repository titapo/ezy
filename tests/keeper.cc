#include <catch.hpp>

#include <ezy/experimental/keeper.h>

using ezy::experimental::owner;
using ezy::experimental::reference_to;

struct Person
{
  int age{0};
};

struct UniquePerson
{
  int age{0};
  UniquePerson(const UniquePerson&) = delete;
  UniquePerson& operator=(const UniquePerson&) = delete;

  UniquePerson(UniquePerson&&) = default;
  UniquePerson& operator=(UniquePerson&&) = default;
};

using ConstPerson = const Person;

SCENARIO("basic behaviour")
{
  GIVEN("an owner")
  {
    ezy::experimental::owner<Person> person{Person{5}};
    static_assert(std::is_same_v<ezy::experimental::detail::keeper_category_t<decltype(person)>, ezy::experimental::owner_category_tag>); // a bit verbose to check: is_owner?
    REQUIRE(person.get().age == 5);
  }
  GIVEN("a reference")
  {
    Person p{6};
    ezy::experimental::reference_to<Person> person{p};
    static_assert(std::is_same_v<ezy::experimental::detail::keeper_category_t<decltype(person)>, ezy::experimental::reference_category_tag>);
    REQUIRE(person.get().age == 6);
  }
}

SCENARIO("construction constraints")
{
  static_assert(!std::is_constructible_v<ezy::experimental::reference_to<Person>, Person&&>);
  static_assert(!std::is_constructible_v<ezy::experimental::reference_to<Person>, Person>);
  static_assert(std::is_constructible_v<ezy::experimental::reference_to<Person>, Person&>);
  static_assert(!std::is_constructible_v<ezy::experimental::reference_to<Person>, const Person&>);

  static_assert(std::is_constructible_v<ezy::experimental::owner<Person>, Person&&>);
  static_assert(std::is_constructible_v<ezy::experimental::owner<Person>, Person>);
  static_assert(!std::is_constructible_v<ezy::experimental::owner<Person>, Person&>);
  static_assert(!std::is_constructible_v<ezy::experimental::owner<Person>, const Person&>);
}

SCENARIO("make_keeper from underlying")
{
  using ezy::experimental::make_keeper;
  static_assert(std::is_same_v<decltype(make_keeper(Person{})), owner<Person>>);
  static_assert(std::is_same_v<decltype(make_keeper(ConstPerson{})), owner<const Person>>);
  const Person jack{7};
  static_assert(std::is_same_v<decltype(make_keeper(jack)), reference_to<const Person>>);
}

SCENARIO("make_keeper from other keeper")
{
  using ezy::experimental::make_keeper;

  owner<Person> owned{Person{}};
  Person p;
  reference_to<Person> referenced{p};

  static_assert(std::is_same_v<decltype(make_keeper(owned)), reference_to<Person>>);

  GIVEN("a moveable owner")
  {
    auto moveable_owner = make_keeper(Person{12});
    static_assert(std::is_same_v<decltype(make_keeper(std::move(moveable_owner))), owner<Person>>);
  }

  static_assert(std::is_same_v<decltype(make_keeper(referenced)), reference_to<Person>>);
  static_assert(std::is_same_v<decltype(make_keeper(std::move(referenced))), reference_to<Person>>);
}

// TODO consider .invoke(...)
SCENARIO("apply")
{
  using ezy::experimental::make_keeper;

  const auto increment_age = [](Person& p) {++p.age;};
  const auto take_person = [](Person&& p) {++p.age; return p; };
  const auto take_person_by_value = [](Person p) {++p.age; return p; };

  owner<Person> owned{Person{10}};
  Person p{20};
  reference_to<Person> referenced{p};

  WHEN("increment age on owner")
  {
    owned.apply(increment_age);
    REQUIRE(owned.get().age == 11);
  }

  WHEN("increment age on reference")
  {
    referenced.apply(increment_age);
    REQUIRE(referenced.get().age == 21);
  }

  WHEN("take from owned")
  {
    auto moveable_owner = make_keeper(Person{15});
    auto taken = std::move(moveable_owner).apply(take_person);
    REQUIRE(taken.age == 16); // does not wraps back automatically
  }

  WHEN("take by value from owned")
  {
    auto copied = owned.apply(take_person_by_value);
    REQUIRE(copied.age == 11);
  }

  WHEN("take by value from owned")
  {
    auto copied = referenced.apply(take_person_by_value);
    REQUIRE(copied.age == 21);
  }
WHEN("take by value from owned -- move")
  {
    auto taken = std::move(owned).apply(take_person_by_value);
    REQUIRE(taken.age == 11);
  }

  WHEN("take by value from owned -- move")
  {
    auto copied = std::move(referenced).apply(take_person_by_value);
    REQUIRE(copied.age == 21);
  }
}

void call_with_owner(owner<Person>) {}
void call_with_owner(owner<UniquePerson>) {}
void call_with_ref(reference_to<Person>) {}
void call_with_ref(reference_to<UniquePerson>) {}
void call_with_const_owner(owner<const Person>) {}
void call_with_const_ref(reference_to<const Person>) {}

SCENARIO("conversions")
{
  owner<Person> owned{Person{10}};
  Person p{20};
  reference_to<Person> referenced{p};

  owner<const Person> owned_const{Person{30}};
  reference_to<const Person> referenced_const{p};

  const owner<Person> const_owned{Person{30}};
  const reference_to<Person> const_referenced{p};

  const owner<const Person> const_owned_const{Person{30}};
  const reference_to<const Person> const_referenced_const{p};

  GIVEN("call_with_owner()")
  {
    WHEN("reference")
    {
      // call_with_owner(referenced); // OK: this does not compile
      call_with_owner(referenced.copy());
    }
    WHEN("owner -> copy")
    {
      call_with_owner(owned.copy());
    }
    WHEN("owner -> move")
    {
      call_with_owner(std::move(owned));
    }
    WHEN("owner -> move then copy")
    {
      //call_with_owner(std::move(owned).copy()); // OK: does not compile: move or copy, pick one
    }

    WHEN("reference to const")
    {
      call_with_owner(referenced_const.mutable_copy());
    }
    WHEN("owned const -> copy")
    {
      call_with_owner(owned_const.mutable_copy());
    }
    WHEN("owned const -> move")
    {
      //call_with_owner(std::move(owned_const)); // OK move from const to mutable is not supported
    }

    WHEN("const reference")
    {
      call_with_owner(referenced.copy()); // OK as well
      call_with_owner(referenced.mutable_copy());
    }
    WHEN("const owned -> copy")
    {
      call_with_owner(const_owned.copy()); // explicit move or copy -> copy
    }
    WHEN("const owned -> move")
    {
      //call_with_owner(std::move(const_owned)); // move from const is not possible at all
    }

    WHEN("const reference to const")
    {
      call_with_owner(const_referenced_const.mutable_copy());
    }
    WHEN("const owned const -> copy")
    {
      call_with_owner(const_owned_const.mutable_copy()); // explicit move or copy -> copy
    }
    WHEN("const owned const -> move")
    {
      //call_with_owner(std::move(const_owned_const)); // move from const is not possible at all
    }
  }

  GIVEN("call_with_ref")
  {
    WHEN("reference")
    {
      call_with_ref(referenced);
      call_with_ref(const_referenced);
      //call_with_ref(referenced_const); // OK
    }

    WHEN("owner")
    {
      //call_with_ref(owned); // does not compile // OK
      call_with_ref(owned.ref()); // explicit ref
      //call_with_ref(owned.copy().ref()); // does not compile // OK
    }
  
  }

  GIVEN("call_with_const_ref")
  {
    WHEN("reference")
    {
      call_with_const_ref(referenced);
    }

    WHEN("owner")
    {
      //call_with_const_ref(owned); // does not compile // OK
      call_with_const_ref(owned.ref()); // explicit ref
      //call_with_const_ref(owned.copy().ref()); // does not compile // OK
    }

    WHEN("const owner")
    {
      call_with_const_ref(const_owned.ref());
    }
  }

  // TODO how to handle/mimic lifetime extension for const&
}
