#include "include/input.h"
#include "include/editor.h"
#include "include/operations.h"
#include "include/utils.h"
#include <cerrno>
#include <cstdlib>
#include <unistd.h>

int readKey() {
  char c = '\0';
  if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
    die("read");
  }
  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1)
      return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1)
      return '\x1b';
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1)
          return '\x1b';
        if (seq[2] == '~') {
          debugLog(seq);
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

void handleInsertMode() {
  int c = readKey();
  switch (c) {
  case CTRL_KEY('q'):
    clearScreen();
    exit(0);
    break;
  case HOME_KEY:
    editorState.cursorx = 0;
    break;
  case END_KEY:
    editorState.cursorx = editorState.screencols - 1;
    break;
  case PAGE_DOWN:
  case PAGE_UP: {
    int times = editorState.screenrows;
    while (times > 0) {
      moveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
      times--;
    }
  } break;
  case ARROW_LEFT:
  case ARROW_RIGHT:
  case ARROW_DOWN:
  case ARROW_UP:
    moveCursor(c);
    break;
  case DEL_KEY: {
    deleteCharAt(editorState.cursory, editorState.cursorx);
  } break;

    // Backspace key
  case 127: {
    deleteCharAt(editorState.cursory, editorState.cursorx - 1);
    if (editorState.cursorx > 0) {
      editorState.cursorx--;
    }
  } break;
    // Escape key
  case 27:
    editorState.mode = NORMAL;
    break;
    // Tab key
  case 9: {
    editorInsertChar('\t');
  } break;
  default:
    if (c >= 32 && c < 127) {
      editorInsertChar(c);
    }
    break;
  }
}
void handleNormalMode() {
  int c = readKey();
  switch (c) {
  case 105:
    editorState.mode = INSERT;
    break;
  case ARROW_LEFT:
  case ARROW_RIGHT:
  case ARROW_DOWN:
  case ARROW_UP:
    moveCursor(c);
    break;
  case 104:
    moveCursor(ARROW_LEFT);
    break;
  case 106:
    moveCursor(ARROW_DOWN);
    break;
  case 107:
    moveCursor(ARROW_UP);
    break;
  case 108:
    moveCursor(ARROW_RIGHT);
    break;
  }
}

void processKeyPress() {
  switch (editorState.mode) {
  case NORMAL:
    handleNormalMode();
    break;
  case INSERT:
    handleInsertMode();
    break;
  case VISUAL:
    break;
  case COMMAND:
    break;
  }
}
