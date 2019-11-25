#include "Message.h"

#include <iostream>
#include <string>

struct Foo {
  int a;
  char b[2];
  float c;
  std::string s;
};

void to_message(ipc::message& m, const Foo& f) { m << f.a << f.b << f.c << f.s; }

void from_message(ipc::message& m, Foo& f) { m >> f.a >> f.b >> f.c >> f.s; }

using namespace ipc;

// void from_message(message& m, Foo& f) { m >> f.a >> f.b >> f.s; }

int main()
{
  const uint32_t strc = reinterpret_cast<const uint32_t&>("cama");
  const uint32_t meth = reinterpret_cast<const uint32_t&>("maca");
  message m(strc, meth);
  m << Foo { 5, { 'c', 'a' }, 4.3, "ma" };
  m << std::string("ma");
  std::cout << "size = " << m.size() << '\n';

  auto data = m.data();
  for (auto i = 0; i < m.size(); i++)
    std::cout << data[i] << ", ";
  std::cout << '\n';

  Foo o;

  m >> o;

  std::cout << o.a << o.b << o.c << o.s << '\n';

  return 0;
}
