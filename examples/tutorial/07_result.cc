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

int main()
{
  auto foo_result = Foo::create(3);
  foo_result.visit(
      [](const Foo& foo) { std::cout << "Foo: " << foo.get_number() << "\n"; },
      [](const ErrorMsg& err) { std::cout << "Error: " << err << "\n"; }
      );

  auto err_result = Foo::create(25);
  err_result.visit(
      [](const Foo& foo) { std::cout << "Foo: " << foo.get_number() << "\n"; },
      [](const ErrorMsg& err) { std::cout << "Error: " << err << "\n"; }
      );

  std::cout << foo_result.map(&Foo::get_number).success_or(0) << "\n";
  std::cout << err_result.map(&Foo::get_number).success_or(0) << "\n";

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



  return 7;
}
