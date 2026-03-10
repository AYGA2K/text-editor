#include "include/utils.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

void clearScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4); // clear the screen
  write(STDOUT_FILENO, "\x1b[H",
        3); // reposition the cursor back up at the top-left corner
}

void die(const char *s) {
  clearScreen();
  perror(s);
  exit(1);
}
