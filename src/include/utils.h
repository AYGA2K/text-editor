#pragma once

#include <fstream>

void clearScreen();
void die(const char *s);

template <typename T>
void debugLog(T value) {
  std::ofstream f("tmp/debug.log", std::ios::app);
  if (f) f << value << '\n';
}
