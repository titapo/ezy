# Creating strong types

Strong typedefs (aka newtype) is one main feature of `ezy`. With them the following bug would be easily fixed.

First of all, check the bug in this code. Supposing that the task is: add 100 money to account identified by 42.

```cpp
  void deposit(int amount, int account_id)
  {
    std::cout << "Add " << amount << " of money to account identified by " << account_id << ".\n";
  }

  void call()
  {
    deposit(42, 100); // oops! swapped parameters
  }
```

Swapping-back the parameters would fix this particular case, but the `deposit` function would be still
error-prone, and the next time it will be called with wrong parameters.

So define strong types to fix the problem:

```cpp
#include <ezy/strong_type.h>

using Money = ezy::strong_type<int, struct MoneyTag>;
using AccountId = ezy::strong_type<int, struct AccountIdTag>;
```

`strong_type` is a class template:
 * first parameter is the underlying type, which will be wrapped
 * the second parameter is a tag type, which is used for distinguish strong types. The only requirement is to
   be unique, otherwise the two strong typedef would result the same type.

With these types, the `deposit` function can be updated to:
```
  void deposit(Money amount, AccountId account_id)
  {
    std::cout << "Add " << amount.get() << " of money to account identified by " << account_id.get() << ".\n";
  }
```
(Note the `.get()` calls which returns the underlying `int`)

```cpp
    //deposit(42, 100); // does not compile anymore
    //deposit(AccountId{42}, Money{100}); // this still does not compile
    deposit(Money{100}, AccountId{42}); // this is OK
```

## Don't get it

So the bug is fixed, but writing `.get()` for every print can be a bit tedious, so make it a bit more
convenient.


```cpp
#include <ezy/strong_type.h>
#include <ezy/features/printable.h>

using Money = ezy::strong_type<int, struct MoneyTag, ezy::features::printable>;
using AccountId = ezy::strong_type<int, struct AccountIdTag, ezy::features::printable>;
```

After the tag type, features can be given for strong type. In this case `printable` allows to print the type
as the underlying type would be printed. And now we can get rid of `.get()` accessors

```cpp
  void deposit(Money amount, AccountId account_id)
  {
    std::cout << "Add " << amount << " of money to account identified by " << account_id << ".\n";
  }
```

At the end of the day, only the parameter types had to be changed in the implementation of `deposit`. And what
about the code which uses this function? Well, the compiler will point to every single call which has to be
updated.

Next: [Iterating containers](03_iterable.md)
