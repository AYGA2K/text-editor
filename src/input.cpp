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

void handlePageKeys(int c) {
  if (c == PAGE_UP) {
    editor.cursory = editor.row_offset;
  } else if (c == PAGE_DOWN) {
    editor.cursory = editor.row_offset + editor.screenrows - 1;
    if (editor.cursory > editor.numrows) {
      editor.cursory = editor.numrows;
    }
  }
  int times = editor.screenrows;
  while (times > 0) {
    Operations::moveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
    times--;
  }
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
    handlePageKeys(c);
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

  case BACKSPACE: {
    const int previousRowIndex =
        editor.cursory - 1 >= 0 ? editor.cursory - 1 : -1;
    if ((editor.cursorx == 0) && (previousRowIndex >= 0)) {
      editor.cursory--;
      editor.cursorx = editor.rows[previousRowIndex].chars.size();
    }
    Operations::deleteCharAt(editor.cursory, editor.cursorx - 1);
    if (editor.cursorx > 0) {
      editor.cursorx--;
    }
  } break;
  case ESCAPE:
    break;
  case ENTER: {
    const int currentRowIndex = editor.cursory;
    // If cursor is at the begenning or after the last char of the line
    if (editor.cursorx == 0 ||
        editor.cursorx >=
            static_cast<int>(editor.rows[currentRowIndex].chars.size()) - 1) {
      EditorRow row = {};
      if (currentRowIndex + 1 < static_cast<int>(editor.rows.size())) {
        editor.rows.insert(editor.rows.begin() + currentRowIndex + 1, row);
      } else {
        editor.rows.push_back(row);
      }
    } else {
      // If the cursor is before the last char in the line
      const EditorRow currentRow = editor.rows[currentRowIndex];

      // Chars from cursor till the end of the line
      const std::string cursorEndRowChars = currentRow.chars.substr(
          editor.cursorx, currentRow.chars.size() - editor.cursorx);

      // Convert cursor position from chars index to render index (accounts for
      // tabs)
      int rx = editor.cxToRx(currentRow.chars, editor.cursorx);

      // Rendred chars from cursor till the end of the line
      const std::string cursorEndRowRender = currentRow.render.substr(rx);

      // Keep chars before cursor in the current line
      editor.rows[currentRowIndex].chars =
          currentRow.chars.substr(0, editor.cursorx);
      editor.rows[currentRowIndex].render = currentRow.render.substr(0, rx);

      // Add a new line with the chars after the cursor
      EditorRow newRow = {};
      newRow.chars = cursorEndRowChars;
      newRow.render = cursorEndRowRender;
      editor.rows.insert(editor.rows.begin() + currentRowIndex + 1, newRow);
    }
    editor.numrows++;
    editor.cursorx = 0;
    editor.cursory++;
  }

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
