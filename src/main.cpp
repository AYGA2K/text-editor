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
  editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");
  while (1) {
    refreshScreen();
    processKeyPress();
  }
  return 0;
}
