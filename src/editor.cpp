#include "include/editor.h"
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

std::string getEditorMode() {
  switch (editorState.mode) {
  case NORMAL:
    return "NORMAL";
  case INSERT:
    return "INSERT";
  case COMMAND:
    return "COMMAND";
  case VISUAL:
    return "VISUAL";
  }
}

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
void editorSetStatusMessage(std::string msg) {
  editorState.message = msg;
  editorState.message_time = std::time(NULL);
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
    return;
  }
  std::ofstream file(editorState.filename);
  if (file.is_open()) {
    const std::string data = rowsToString();
    file << data;
    file.close();
    editorSetStatusMessage(std::to_string(data.size()) +
                           " bytes written to disk");
  }
}
