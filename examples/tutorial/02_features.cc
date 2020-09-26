#include <ezy/strong_type.h>
#include <ezy/features/printable.h>
#include <ezy/features/arithmetic.h>

#include <iostream>
namespace ns1
{
  using Integer = ezy::strong_type<int, struct IntegerTag,
        ezy::features::printable,
        ezy::features::addable,
        ezy::features::subtractable
        >;

  void entry()
  {
    const Integer a{5};
    const Integer b{15};
    const Integer c{10};

    std::cout << a + b - c << "\n";

    Integer d{5};
    d += Integer{3};
    d -= Integer{10};

    std::cout << d << "\n";
  }
}

namespace ns2
{
  using Integer = ezy::strong_type<int, struct IntegerTag,
        ezy::features::printable,
        ezy::features::additive,
        ezy::features::equal_comparable
        >;

  void entry()
  {
    Integer a{5};
    a += Integer{10};
    std::cout << std::boolalpha << (a == Integer{15}) << "\n";
    std::cout << std::boolalpha << (a != Integer{15}) << "\n";
  }
}

namespace ns3
{
  using Integer = ezy::strong_type<int, struct IntegerTag,
        ezy::features::printable,
        ezy::features::additive,
        ezy::features::multiplicative,
        ezy::features::equal_comparable
        >;

  void entry()
  {
    Integer a{5};
    a += Integer{10} * Integer{5};
    a /= Integer{11};
    std::cout << std::boolalpha << a << "\n";
  }
}

int main()
{
  ns1::entry();
  ns2::entry();
  // TODO equal comparable can generate != from ==
  ns3::entry();
  return 2;
}
