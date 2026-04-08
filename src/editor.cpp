#include "include/editor.h"
#include <cstddef>
#include <ctime>
#include <fstream>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
#include <unistd.h>

Editor editor;

int Editor::getWindowSize(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

std::string Editor::expandTabs(std::string_view s) {
  std::string result;
  for (char c : s) {
    if (c == '\t')
      result.append(tabWidth, ' ');
    else
      result += c;
  }
  return result;
}

void Editor::updateRow(EditorRow &row) { row.render = expandTabs(row.chars); }

void Editor::open(const std::string &fname) {
  std::ifstream file(fname);
  if (!file.is_open()) {
    Utils::die("fopen");
  }
  filename = fname;
  std::string line;
  while (std::getline(file, line)) {
    EditorRow row;
    row.chars = line; // keep tabs as '\t' in chars
    updateRow(row);   // prepare render with tabs expanded
    rows.push_back(row);
    numrows += 1;
  }
}

void Editor::init() {
  cursorx = 0;
  cursory = 0;
  numrows = 0;
  row_offset = 0;
  col_offset = 0;
  message_time = std::time(NULL);
  if (getWindowSize(&screenrows, &screencols) == -1) {
    Utils::die("getWindowSize");
  }
  screencols -= gutterWidth;
  screenrows -= 2;
}

void Editor::setStatusMessage(std::string_view msg) {
  message = msg;
  message_time = std::time(NULL);
}

void Editor::quit() {
  Utils::clearScreen();
  if (modified && quitTimes == 0) {
    setStatusMessage(
        "File has unsaved data! please click Ctrl-q one more time to quit");
    quitTimes++;
    return;
  }
  exit(0);
}

std::string Editor::rowsToString() {
  std::string result;
  for (EditorRow row : rows) {
    result.append(row.chars);
    result.append("\n");
  }
  return result;
}

void Editor::save() {
  if (filename.empty()) {
    filename = prompt("Save as: ", NULL);
    if (filename.empty()) {
      setStatusMessage("Save canceled");
    }
  }
  if (filename.empty()) {
    return;
  }
  std::ofstream file(filename);
  if (file.is_open()) {
    const std::string data = rowsToString();
    file << data;
    file.close();
    setStatusMessage(std::to_string(data.size()) + " bytes written to disk");
    modified = false;
  }
}

int Editor::cxToRx(std::string_view chars, int cx) {
  int rx = 0;
  for (int i = 0; i < cx; i++) {
    if (chars[i] == '\t') {
      rx += tabWidth;
    } else {
      rx++;
    }
  }
  return rx;
}

std::string Editor::prompt(const std::string &promptMsg,
                           void (*callback)(std::string, int)) {
  std::string buff = "";
  while (true) {
    setStatusMessage(promptMsg + buff);
    Output::refreshScreen();
    int c = Input::readKey();
    if (c == ESCAPE) {
      setStatusMessage("");
      if (callback) {
        callback(buff, c);
      }
      return "";
    }
    if ((c == BACKSPACE || c == DEL_KEY) && !buff.empty()) {
      buff.pop_back();
    }
    if (c == '\r' && buff.size() != 0) {
      setStatusMessage("");
      if (callback) {
        callback(buff, c);
      }
      return buff;
    } else if (!iscntrl(c) && c < 128) {
      buff += c;
    }
    if (callback) {
      callback(buff, c);
    }
  }
}

void findCallback(std::string query, int key) {
  static int last_row = editor.cursory;
  static int last_column = 0;
  static int start = 0;     // from which column in the row we start searching
  static int direction = 1; // 1 = forward, -1 = backward

  // Exit search mode
  if (key == ENTER || key == ESCAPE) {
    last_row = -1;
    direction = 1;
    return;
  }

  // Set direction and start position based on arrow key
  if (key == ARROW_RIGHT || key == ARROW_DOWN) {
    if (editor.cursory == last_row) {
      // Same row: start after current match
      start = editor.cursorx + 1;
    } else {
      // Different row: start from beginning
      start = 0;
    }
    direction = 1;
  } else if (key == ARROW_LEFT || key == ARROW_UP) {
    if (editor.cursory == last_row) {
      // Same row: start before current match
      start = last_column - 1;
      if (start < 0)
        start = 0; // prevent negative
    } else {
      // Different row: start from end of string (for reverse search)
      start = static_cast<int>(std::string::npos);
    }
    direction = -1;
  } else {
    // Any other key resets search state
    last_row = -1;
    direction = 1;
  }

  if (last_row == -1) {
    direction = 1;
  }

  int current = last_row;
  // Search until we find a match (wrap around if needed)
  for (int i = 0; i < editor.numrows; i++) {
    // Move to next/previous row when we exhaust current row
    if ((direction == 1 && start == 0 && last_column == 0) ||
        (direction == -1 && start == std::string::npos && last_column == 0)) {
      current += direction;
    }

    // Wrap around
    if (current == -1) {
      current = editor.numrows - 1;
    } else if (current == editor.numrows) {
      current = 0;
    }

    const EditorRow &row = editor.rows[current];
    int pos = -1;

    if (direction == 1) {
      // Forward search
      size_t found = row.chars.find(query, start);
      if (found != std::string::npos)
        pos = static_cast<int>(found);
    } else {
      // Backward search
      size_t found;
      if (start == std::string::npos) {
        found = row.chars.rfind(query); // start from end
      } else {
        found = row.chars.rfind(query, start); // start before current
      }
      if (found != std::string::npos)
        pos = static_cast<int>(found);
    }

    if (pos != -1) {
      // Match found
      last_row = current;
      editor.cursory = current;
      editor.cursorx = pos;
      last_column = pos;
      break;
    } else {
      // No match in this row: reset for next row
      if (direction == 1) {
        start = 0;
      } else {
        start = static_cast<int>(std::string::npos);
      }
      last_column = 0;
    }
  }
}

void Editor::find() {
  int savedCursorX = cursorx;
  int savedCursorY = cursory;
  int saved_colloff = col_offset;
  int saved_rowoff = row_offset;
  const std::string query = prompt("Search: ", findCallback);
  if (query.empty()) {
    return;
  }
  cursorx = savedCursorX;
  cursory = savedCursorY;
  col_offset = saved_colloff;
  row_offset = saved_rowoff;
}
