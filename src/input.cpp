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

void handlePageDownPageUpKeys(int c) {
  if (c == PAGE_UP) {
    editorState.cursory = editorState.row_offset;
  } else if (c == PAGE_DOWN) {
    editorState.cursory = editorState.row_offset + editorState.screenrows - 1;
    if (editorState.cursory > editorState.numrows) {
      editorState.cursory = editorState.numrows;
    }
  }
  int times = editorState.screenrows;
  while (times > 0) {
    moveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
    times--;
  }
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
    if (editorState.cursory < editorState.numrows) {
      editorState.cursorx = editorState.rows[editorState.cursory].chars.size();
    }
    break;
  case PAGE_DOWN:
  case PAGE_UP:
    handlePageDownPageUpKeys(c);
    break;
  case ARROW_LEFT:
  case ARROW_RIGHT:
  case ARROW_DOWN:
  case ARROW_UP:
    moveCursor(c);
    break;
  case DEL_KEY: {
    deleteCharAt(editorState.cursory, editorState.cursorx);
  } break;

  case BACKSPACE: {
    deleteCharAt(editorState.cursory, editorState.cursorx - 1);
    if (editorState.cursorx > 0) {
      editorState.cursorx--;
    }
  } break;
  case ESCAPE:
    editorState.mode = NORMAL;
    break;
  case TAB: {
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
  case PAGE_DOWN:
  case PAGE_UP:
    handlePageDownPageUpKeys(c);
    break;
  case ':':
    editorState.mode = COMMAND;
    break;
  }
}
void handleCommandMode() {
  int c = readKey();
  switch (c) {
  case 'w':
    editorSave();
    break;
  case 'q':
    clearScreen();
    exit(0);
  case ESCAPE:
    editorState.mode = NORMAL;
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
    handleCommandMode();
    break;
  }
}
