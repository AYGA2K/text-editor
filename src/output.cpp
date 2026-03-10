#include "include/output.h"
#include "include/editor.h"
#include <sstream>
#include <string>
#include <unistd.h>

void refreshScreen() {
  editorScroll();
  buffer += "\x1b[H"; // reposition the cursor back up at the top-left corner

  drawRaws();

  std::ostringstream oss;
  oss << "\x1b[" << editorConfig.cursory - editorConfig.row_offest + 1 << ";"
      << editorConfig.cursorx - editorConfig.col_offset + 1
      << "H"; // reposition the cursor
  buffer += oss.str();

  write(STDOUT_FILENO, buffer.data(), buffer.size());

  buffer.clear();
}

void drawRaws() {
  for (int y = 0; y < editorConfig.screenrows; y++) {
    int filerow = y + editorConfig.row_offest;
    if (filerow >= editorConfig.numrows) {
      // Draw welcome message in the middle
      if (editorConfig.numrows == 0 && y == editorConfig.screenrows / 3) {
        std::string welcome = "The editor";
        int welcomelen = static_cast<int>(welcome.size());
        if (welcomelen > editorConfig.screencols)
          welcomelen = editorConfig.screencols;

        int padding = (editorConfig.screencols - welcomelen) / 2;
        if (padding > 0) {
          buffer += "~";
          padding--;
        }
        buffer += std::string(padding, ' ');
        buffer += welcome.substr(0, welcomelen);
      } else {
        buffer += "~";
      }
    } else {
      const std::string &line = editorConfig.rows[filerow];
      int linelen = line.size();
      int len;

      if (editorConfig.col_offset >= linelen) {
        len = 0;
      } else {
        len = linelen - editorConfig.col_offset;
        if (len > editorConfig.screencols) {
          len = editorConfig.screencols;
        }
      }

      buffer += line.substr(editorConfig.col_offset, len);
    }

    buffer += "\x1b[K"; // clear the rest of the line

    if (y < editorConfig.screenrows - 1) {
      buffer += "\r\n"; // newline
    }
  }
}

void editorScroll() {
  if (editorConfig.cursory < editorConfig.row_offest) {
    editorConfig.row_offest = editorConfig.cursory;
  }

  if (editorConfig.cursory >=
      editorConfig.row_offest + editorConfig.screenrows) {
    editorConfig.row_offest =
        editorConfig.cursory - editorConfig.screenrows + 1;
  }
  if (editorConfig.cursorx < editorConfig.col_offset) {
    editorConfig.col_offset = editorConfig.cursorx;
  }
  if (editorConfig.cursorx >=
      editorConfig.col_offset + editorConfig.screencols) {
    editorConfig.col_offset =
        editorConfig.cursorx - editorConfig.screencols + 1;
  }
}
