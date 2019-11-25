#pragma once
#include "data.hpp"

#include <chrono>

namespace example {

class time_facade {
  time_facade();

public:
  void do_something();
  std::chrono::system_clock::time_point time_in(const location &);
  static time_facade &instance();
};

} // namespace example
