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

void Editor::findCallback(std::string query, int key) {
  static int last_match = -1;
  static int direction = 1;
  if (key == ENTER || key == ESCAPE) {
    last_match = -1;
    direction = 1;
    return;
  }
  if (key == ARROW_RIGHT || key == ARROW_DOWN) {
    direction = 1;
  } else if (key == ARROW_LEFT || key == ARROW_UP) {
    direction = -1;
  } else {
    last_match = -1;
    direction = 1;
  }
  if (last_match == -1) {
    direction = 1;
  }
  int current = last_match;
  for (int i = 0; i < editor.numrows; i++) {
    current += direction;
    if (current == -1) {
      current = editor.numrows - 1;
    } else if (current == editor.numrows) {
      current = 0;
    }
    const EditorRow &row = editor.rows[current];
    const int pos = row.chars.find(query);
    if (pos != static_cast<int>(std::string::npos)) {
      last_match = current;
      editor.cursory = current;
      editor.cursorx = pos;
      editor.row_offset = editor.numrows;
      break;
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
