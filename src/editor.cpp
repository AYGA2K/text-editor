#include "include/editor.h"
#include "include/utils.h"
#include <fstream>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

std::string buffer;
struct EditorConfig editorConfig;

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
    editorConfig.rows.push_back(line);
    editorConfig.numrows += 1;
  }
}

void initEditor() {
  editorConfig.cursorx = 0;
  editorConfig.cursory = 0;
  editorConfig.numrows = 0;
  editorConfig.row_offest = 0;
  editorConfig.col_offset = 0;
  if (getWindowSize(&editorConfig.screenrows, &editorConfig.screencols) == -1) {
    die("getWindowSize");
  }
}
