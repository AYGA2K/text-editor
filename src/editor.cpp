#include "include/editor.h"
#include "include/input.h"
#include "include/output.h"
#include "include/utils.h"
#include <cstddef>
#include <ctime>
#include <fstream>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
#include <unistd.h>

std::string buffer;
struct EditorState editorState;
int tabWidth = 4;
int quitTimes = 0;

int getWindowSize(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

std::string expandTabs(std::string_view s) {
  std::string result;
  for (char c : s) {
    if (c == '\t')
      result.append(tabWidth, ' ');
    else
      result += c;
  }
  return result;
}

void editorUpdateRow(EditorRow &row) { row.render = expandTabs(row.chars); }

void editorOpen(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    die("fopen");
  }
  editorState.filename = filename;
  std::string line;
  while (std::getline(file, line)) {
    EditorRow row;
    row.chars = line;     // keep tabs as '\t' in chars
    editorUpdateRow(row); // prepare render with tabs expanded
    editorState.rows.push_back(row);
    editorState.numrows += 1;
  }
}

void initEditor() {
  editorState.cursorx = 0;
  editorState.cursory = 0;
  editorState.numrows = 0;
  editorState.row_offset = 0;
  editorState.col_offset = 0;
  editorState.message_time = std::time(NULL);
  if (getWindowSize(&editorState.screenrows, &editorState.screencols) == -1) {
    die("getWindowSize");
  }
  editorState.screenrows -= 2;
}
void editorSetStatusMessage(std::string_view msg) {
  editorState.message = msg;
  editorState.message_time = std::time(NULL);
}
void editorQuit() {
  clearScreen();
  if (editorState.modified && quitTimes == 0) {
    editorSetStatusMessage(
        "File has unsaved data! please click Ctrl-q one more time to quit");
    quitTimes++;
    return;
  }
  exit(0);
}

std::string rowsToString() {
  std::string result;
  for (EditorRow row : editorState.rows) {
    result.append(row.chars);
    result.append("\n");
  }
  return result;
}

void editorSave() {
  if (editorState.filename.empty()) {
    editorState.filename = editorPrompt("Save as: ", NULL);
    if (editorState.filename.empty()) {
      editorSetStatusMessage("Save canceled");
    }
  }
  if (editorState.filename.empty()) {
    return;
  }
  std::ofstream file(editorState.filename);
  if (file.is_open()) {
    const std::string data = rowsToString();
    file << data;
    file.close();
    editorSetStatusMessage(std::to_string(data.size()) +
                           " bytes written to disk");
    editorState.modified = false;
  }
}

int cxToRx(std::string_view chars, int cx) {
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
std::string editorPrompt(const std::string &prompt,
                         void (*callback)(std::string, int)) {
  std::string buff = "";
  while (true) {
    editorSetStatusMessage(prompt + buff);
    refreshScreen();
    int c = readKey();
    if (c == ESCAPE) {
      editorSetStatusMessage("");
      if (callback) {
        callback(buff, c);
      }
      return "";
    }
    if ((c == BACKSPACE || c == DEL_KEY) && !buff.empty()) {
      buff.pop_back();
    }
    if (c == '\r' && buff.size() != 0) {
      editorSetStatusMessage("");
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
void editorFindCallback(std::string query, int key) {
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
  for (int i = 0; i < editorState.numrows; i++) {
    current += direction;
    if (current == -1) {
      current = editorState.numrows - 1;
    } else if (current == editorState.numrows) {
      current = 0;
    }
    const EditorRow &row = editorState.rows[current];
    const int pos = row.chars.find(query);
    if (pos != std::string::npos) {
      last_match = current;
      editorState.cursory = current;
      editorState.cursorx = pos;
      editorState.row_offset = editorState.numrows;
      break;
    }
  }
}
void editorFind() {
  int savedCursorX = editorState.cursorx;
  int savedCursorY = editorState.cursory;
  int saved_colloff = editorState.col_offset;
  int saved_rowoff = editorState.row_offset;
  const std::string query = editorPrompt("Search: ", editorFindCallback);
  if (query.empty()) {
    return;
  }
  editorState.cursorx = savedCursorX;
  editorState.cursory = savedCursorY;
  editorState.col_offset = saved_colloff;
  editorState.row_offset = saved_rowoff;
}
