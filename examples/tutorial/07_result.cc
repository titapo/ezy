#include <ezy/result>
#include <ezy/string.h>

#include <iostream>
#include <string>

using ErrorMsg = std::string;

class Foo
{
  public:
    static ezy::result<Foo, ErrorMsg> create(int n)
    {
      if (n > 20)
      {
        return ErrorMsg{"Too big!"};
      }

      return Foo(n);
    }

    int get_number() const
    {
      return number;
    }

  private:
    explicit Foo(int n)
      : number(n)
    {}

    const int number{0};
};

auto print_line = [](const auto& i) {
  std::cout << i << "\n";
};

#include <ezy/algorithm.h>
#include <cctype>

void factory()
{
  const auto foo_result = Foo::create(3);
  foo_result.visit(
      [](const Foo& foo) { std::cout << "Foo: " << foo.get_number() << "\n"; },
      [](const ErrorMsg& err) { std::cout << "Error: " << err << "\n"; }
      );

  const auto err_result = Foo::create(25);
  err_result.visit(
      [](const Foo& foo) { std::cout << "Foo: " << foo.get_number() << "\n"; },
      [](const ErrorMsg& err) { std::cout << "Error: " << err << "\n"; }
      );

  std::cout << foo_result.map(&Foo::get_number).success_or(0) << "\n";
  std::cout << err_result.map(&Foo::get_number).success_or(0) << "\n";
  std::cout << foo_result.map_or(&Foo::get_number, 0) << "\n";
  std::cout << err_result.map_or(&Foo::get_number, 0) << "\n";

  auto to_upper = [](const std::string& s)
  {
    return ezy::collect<std::string>(ezy::transform(s, [](auto ch) {return std::toupper(ch); }));
  };

  std::cout << err_result.map_error(to_upper).error() << "\n";

  std::cout << "---\n";

  /*
   ErrorMsg is just a (not-strong) typedef for string: visit does work because of ambiguity
  foo_result.map(&Foo::get_number).map(ezy::to_string).visit(
      [](const std::string& s) { std::cout << "Foo: " << s << "\n"; },
      [](const ErrorMsg& err) { std::cout << "Error: " << err << "\n"; }
      );

  err_result.map(&Foo::get_number).map(ezy::to_string).visit(
      [](const std::string& s) { std::cout << "Foo: " << s << "\n"; },
      [](const ErrorMsg& err) { std::cout << "Error: " << err << "\n"; }
      );
  */

  // map_or_else for side effect???
  foo_result.map(&Foo::get_number).map(ezy::to_string).map_or_else(
      [](const std::string& s) { std::cout << "from Foo: " << s << "\n"; },
      [](const ErrorMsg& err) { std::cout << "from Error: " << err << "\n"; }
      );
  err_result.map(&Foo::get_number).map(ezy::to_string).map_or_else(
      [](const std::string& s) { std::cout << "from Foo: " << s << "\n"; },
      [](const ErrorMsg& err) { std::cout << "from Error: " << err << "\n"; }
      );

}

ezy::result<int, ErrorMsg> to_int(const std::string& str)
{
  try
  {
    return {std::stoi(str)};
  }
  catch (const std::exception& ex)
  {
    return {ex.what()};
  }
}

namespace v1
{
  ezy::result<std::string, std::string>
  divide(const std::string& s1, const std::string& s2)
  {
    std::cout << s1 << " / " << s2 << " = ?\n";
    const auto a = to_int(s1);
    const auto b = to_int(s2);

    const auto c = a.and_then(
        [&b](auto x) {
          return b.map([&x](auto y) { return x / y;});
        });

    return c.map(ezy::to_string);
  }
}

#include <ezy/experimental/function>
namespace v2
{
  ezy::result<std::string, std::string>
  divide(const std::string& s1, const std::string& s2)
  {
    std::cout << s1 << " / " << s2 << " = ?\n";
    const auto a = to_int(s1);
    const auto b = to_int(s2);

    static constexpr auto div = ezy::experimental::curry(std::divides<>{});

    const auto c = a.and_then(
        [&b](auto x) {
          return b.map(div(x));
        }
        );

    return c.map(ezy::to_string);
  }
}

namespace v3
{
  constexpr auto safe_divides = [](int x, int y) -> ezy::result<int, ErrorMsg>
  {
    if (y == 0)
      return {"division by zero"};
    else
      return {x / y};
  };

  ezy::result<std::string, std::string>
  divide(const std::string& s1, const std::string& s2)
  {
    std::cout << s1 << " / " << s2 << " =  ";
    const auto a = to_int(s1);
    const auto b = to_int(s2);

    static constexpr auto div = ezy::experimental::curry(safe_divides);

    const auto c = a.and_then(
        [&b](auto x) {
          return b.and_then(div(x));
        }
        );

    return c.map(ezy::to_string);
  }
}

void instead_of_exceptions()
{
  print_line("-----");
  to_int("123").visit(print_line);
  to_int("hello").visit(print_line);
  to_int("123345678948472628").visit(print_line);

  using namespace v3;

  divide("120", "4").visit(print_line);
  divide("1", "4").visit(print_line);
  divide("hundred", "4").visit(print_line);
  divide("120", "four").visit(print_line);
  divide("10", "0").visit(print_line);
}

int main()
{
  factory();

  instead_of_exceptions();

  return 7;
}
