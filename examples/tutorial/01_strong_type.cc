#include <ezy/strong_type.h>
#include <iostream>


namespace buggy
{
  void deposit(int amount, int account_id)
  {
    std::cout << "Add " << amount << " of money to account identified by " << account_id << ".\n";
  }

  void call()
  {
    deposit(42, 100); // oops! swapped parameters
  }
}

namespace fixed
{
  using Money = ezy::strong_type<int, struct MoneyTag>;
  using AccountId = ezy::strong_type<int, struct AccountIdTag>;

  void deposit(Money amount, AccountId account_id)
  {
    std::cout << "Add " << amount.get() << " of money to account identified by " << account_id.get() << ".\n";
  }

  void call()
  {
    //deposit(42, 100); // does not compile anymore
    //deposit(AccountId{42}, Money{100}); // this still does not compile
    deposit(Money{100}, AccountId{42}); // this is OK
  }
}

#include <ezy/features/printable.h>
namespace convenient
{
  using Money = ezy::strong_type<int, struct MoneyTag, ezy::features::printable>;
  using AccountId = ezy::strong_type<int, struct AccountIdTag, ezy::features::printable>;

  void deposit(Money amount, AccountId account_id)
  {
    std::cout << "Add " << amount << " of money to account identified by " << account_id << ".\n";
  }

  void call()
  {
    deposit(Money{100}, AccountId{42}); // this is OK
  }
}


int main()
{
  // Task: Add 100 money to account 42
  buggy::call();
  fixed::call();
  convenient::call();
  return 0;
}
