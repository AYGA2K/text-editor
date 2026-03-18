#include "include/editor.h"
#include "include/utils.h"
#include <fstream>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
#include <unistd.h>

std::string buffer;
struct EditorState editorState;
int tabWidth = 4;

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
  if (getWindowSize(&editorState.screenrows, &editorState.screencols) == -1) {
    die("getWindowSize");
  }
  editorState.screenrows -= 1;
}
