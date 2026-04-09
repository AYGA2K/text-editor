#include "include/editor.h"
#include <cstdio>
#include <cstdlib>
#include <string_view>
#include <sys/types.h>
#include <unistd.h>

namespace Utils {

void clearScreen() {
  ssize_t n = write(STDOUT_FILENO, "\x1b[2J", 4); // clear the screen
  if (n == -1) {
    die("Error writing to STDOUT_FILENO");
  }
  n = write(STDOUT_FILENO, "\x1b[H",
            3); // reposition the cursor back up at the top-left corner

  if (n == -1) {
    die("Error writing to STDOUT_FILENO");
  }
}

void die(std::string_view s) {
  clearScreen();
  perror(s.data());
  exit(1);
}
} // namespace Utils
