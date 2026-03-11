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
  oss << "\x1b[" << editorState.cursory - editorState.row_offest + 1 << ";"
      << editorState.cursorx - editorState.col_offset + 1
      << "H"; // reposition the cursor
  buffer += oss.str();

  write(STDOUT_FILENO, buffer.data(), buffer.size());

  buffer.clear();
}

void drawRaws() {
  for (int y = 0; y < editorState.screenrows; y++) {
    int filerow = y + editorState.row_offest;
    if (filerow >= editorState.numrows) {
      // Draw welcome message in the middle
      if (editorState.numrows == 0 && y == editorState.screenrows / 3) {
        std::string welcome = "The editor";
        int welcomelen = static_cast<int>(welcome.size());
        if (welcomelen > editorState.screencols)
          welcomelen = editorState.screencols;

        int padding = (editorState.screencols - welcomelen) / 2;
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
      const std::string &line = editorState.rows[filerow];
      int linelen = line.size();
      int len;

      if (editorState.col_offset >= linelen) {
        len = 0;
      } else {
        len = linelen - editorState.col_offset;
        if (len > editorState.screencols) {
          len = editorState.screencols;
        }
      }

      buffer += line.substr(editorState.col_offset, len);
    }

    buffer += "\x1b[K"; // clear the rest of the line

    if (y < editorState.screenrows - 1) {
      buffer += "\r\n"; // newline
    }
  }
}

void editorScroll() {
  if (editorState.cursory < editorState.row_offest) {
    editorState.row_offest = editorState.cursory;
  }

  if (editorState.cursory >=
      editorState.row_offest + editorState.screenrows) {
    editorState.row_offest =
        editorState.cursory - editorState.screenrows + 1;
  }
  if (editorState.cursorx < editorState.col_offset) {
    editorState.col_offset = editorState.cursorx;
  }
  if (editorState.cursorx >=
      editorState.col_offset + editorState.screencols) {
    editorState.col_offset =
        editorState.cursorx - editorState.screencols + 1;
  }
}
