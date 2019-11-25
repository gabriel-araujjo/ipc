#pragma once
#include "data.hpp"

namespace example {

enum mouse_button {
  LEFT,
  RIGTH,
  MIDDLE,
};

// Testing may listener approaches
struct on_mouse_click {
  virtual void on_press(point, mouse_button) = 0;
  virtual void on_release(point, mouse_button) = 0;
};

struct mouse_move_listener {
  virtual void operator()(point, const int *dx, int* const dy, int const dz) = 0;
};

struct mouse_facade {
  bool set_click_listener(const on_mouse_click *);
  bool set_move_listener(const mouse_move_listener *);
  static mouse_facade &instance();

private:
  mouse_facade();
};

} // namespace example
