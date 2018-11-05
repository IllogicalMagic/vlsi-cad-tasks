#ifndef STEINER_SUPPORT_H_DEFINED__
#define STEINER_SUPPORT_H_DEFINED__

#include <iostream>

#include <cstdlib>

template<typename... Args>
[[noreturn]] void report_error(Args&&... args) {
  (std::cerr << ... << args);
  exit(1);
}

#endif
