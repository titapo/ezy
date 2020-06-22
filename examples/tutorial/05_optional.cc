#include <ezy/optional>

ezy::optional<int> even(int i)
{
  if (i % 2 == 0)
    return {i};
  else
    return std::nullopt;
}

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


  print_line(from_ten.value_or(100));
  print_line(from_eleven.value_or(100));


  return 5;
}
