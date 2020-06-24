#include <ezy/optional>
#include <ezy/string.h>

ezy::optional<int> even(int i)
{
  if (i % 2 == 0)
    return i;
  else
    return std::nullopt;
}
// explicit constructible, because extended type

#include <iostream>

int main()
{
  //1.
  const auto from_ten = even(10);
  std::cout << from_ten.value() << "\n";

  //2.
  // BAD
  const auto from_eleven = even(11);
  //std::cout << from_eleven.value() << "\n";

  if (from_eleven.has_value())
  {
    std::cout << from_eleven.value() << "\n";
  }

  //3.
  auto print_line = [](auto&& i) {
    std::cout << i << "\n";
  };

  // does not work now
  //from_ten.map(print_line);
  //from_eleven.inspect(print_line);
  //from_eleven.tee(print_line);


  print_line(even(10).value_or(-100));
  print_line(even(11).value_or(-100));

  /*
   value_or

    int --------+-------> int
               /
    nullopt --+


   int f(int)
   map(f)

    int ------> f() ----> int

    nullopt ------------> nullopt


   string f(int)
   map(f)

    int ------> f() ------> string

    nullopt --------------> nullopt


   optional<T> f(int)
   and_then(h)

    int ------> h() ----> T
                  \
    nullopt -------+----> nullopt
  */

  //4.
  print_line(from_ten.map([](int i) {return i + 10;}).value_or(-100));
  print_line(from_eleven.map([](int i) {return i + 10;}).value_or(-100));

  //5.
  auto half = [](int i){ return i / 2; };
  print_line(from_ten.map(half).value_or(-100));
  print_line(from_eleven.map(half).value_or(-100));

  print_line(from_ten.map(half).and_then(even).value_or(-100));
  print_line(from_eleven.map(half).and_then(even).value_or(-100));
  print_line(even(16).map(half).and_then(even).value_or(-100));

  /*
     even(16).map(half).and_then(even).map_or(ezy::to_string, "none")


              even(16)   .  map(half)    . and_then(even) . map_or(ezy::to_string, "none");

          |              |               |                |                                |

    int -----> even() --------> half() -----> even() ---------> to_string ---------+---------> string
                    \                              \                              /
            nullopt  +------------------------------+----------------------------+

   */

  //6.
  print_line(even(10).map(ezy::to_string).value_or("none"));
  print_line(even(11).map(ezy::to_string).value_or("none"));

  print_line(even(10).map_or(ezy::to_string, "none"));
  print_line(even(11).map_or(ezy::to_string, "none"));

  //
  // optional references are not supported
  // link to ezy::pointer



  return 5;
}
