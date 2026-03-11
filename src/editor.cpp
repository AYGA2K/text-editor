#include "include/editor.h"
#include "include/utils.h"
#include <fstream>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

std::string buffer;
struct EditorState editorState;

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

void editorOpen(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    die("fopen");
  }
  std::string line;
  while (std::getline(file, line)) {
    editorState.rows.push_back(line);
    editorState.numrows += 1;
  }
}

void initEditor() {
  editorState.cursorx = 0;
  editorState.cursory = 0;
  editorState.numrows = 0;
  editorState.row_offest = 0;
  editorState.col_offset = 0;
  if (getWindowSize(&editorState.screenrows, &editorState.screencols) == -1) {
    die("getWindowSize");
  }
}
