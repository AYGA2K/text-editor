#include "include/editor.h"
#include <unistd.h>

namespace Input {

int readKey() {
  char c = '\0';
  ssize_t nread = 0;
  while ((nread = read(STDIN_FILENO, &c, 1)) == 0)
    ;
  if (nread == -1) {
    Utils::die("read");
  }
  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) == 0)
      return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) == 0)
      return '\x1b';
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) == 0)
          return ESCAPE;
        if (seq[2] == '~') {
          switch (seq[1]) {
          case '1':
            return HOME_KEY;
          case '3':
            return DEL_KEY;
          case '4':
            return END_KEY;
          case '5':
            return PAGE_UP;
          case '6':
            return PAGE_DOWN;
          case '7':
            return HOME_KEY;
          case '8':
            return END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
        case 'A':
          return ARROW_UP;
        case 'B':
          return ARROW_DOWN;
        case 'C':
          return ARROW_RIGHT;
        case 'D':
          return ARROW_LEFT;
        case 'H':
          return HOME_KEY;
        case 'F':
          return END_KEY;
        }
      }
    } else if (seq[0] == 'O') {
      switch (seq[1]) {
      case 'H':
        return HOME_KEY;
      case 'F':
        return END_KEY;
      }
    }
    return '\x1b';
  }
  return c;
}

void processKeyPress() {
  int c = readKey();
  switch (c) {
  case CTRL_KEY('q'):
    editor.quit();
    break;
  case CTRL_KEY('s'):
    editor.save();
    break;
  case CTRL_KEY('f'):
    editor.find();
    break;
  case HOME_KEY:
    editor.cursorx = 0;
    break;
  case END_KEY:
    if (editor.cursory < editor.numrows) {
      editor.cursorx = editor.rows[editor.cursory].chars.size();
    }
    break;
  case PAGE_DOWN:
  case PAGE_UP:
    Operations::handlePageKeys(c);
    break;
  case ARROW_LEFT:
  case ARROW_RIGHT:
  case ARROW_DOWN:
  case ARROW_UP:
    Operations::moveCursor(c);
    break;
  case DEL_KEY: {
    Operations::deleteCharAt(editor.cursory, editor.cursorx);
  } break;

  case BACKSPACE:
    Operations::handleBackspace();
    break;
  case ESCAPE:
    break;
  case ENTER:
    Operations::handleEnter();
    break;
  case TAB: {
    Operations::insertChar('\t');
  } break;
  default:
    if (c >= 32 && c < 127) {
      Operations::insertChar(c);
    }
    break;
  }
}
} // namespace Input
