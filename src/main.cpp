#include "include/editor.h"
#include "include/input.h"
#include "include/output.h"
#include "include/terminal.h"
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  enableRawMode();
  initEditor();
  if (argc >= 2) {
    editorOpen(argv[1]);
  }
  while (1) {
    refreshScreen();
    processKeyPress();
  }
  return 0;
}
