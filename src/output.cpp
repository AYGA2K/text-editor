#include "include/output.h"
#include "include/editor.h"
#include <cstddef>
#include <sstream>
#include <string>
#include <unistd.h>

void refreshScreen() {
  editorScroll();
  buffer += "\x1b[H"; // reposition the cursor back up at the top-left corner

  drawRaws();

  int rx = 0;
  if (editorState.cursory < editorState.numrows) {
    const EditorRow &row = editorState.rows[editorState.cursory];
    for (int j = 0;
         j < editorState.cursorx && j < static_cast<int>(row.chars.size());
         j++) {
      if (row.chars[j] == '\t') {
        rx += (tabWidth - 1) - (rx % tabWidth) + 1;
      } else {
        rx++;
      }
    }
  }

  std::ostringstream oss;
  oss << "\x1b[" << editorState.cursory - editorState.row_offset + 1 << ";"
      << rx - editorState.col_offset + 1 << "H"; // reposition the cursor
  buffer += oss.str();

  write(STDOUT_FILENO, buffer.data(), buffer.size());

  buffer.clear();
}

void drawRaws() {
  for (int y = 0; y < editorState.screenrows; y++) {
    int filerow = y + editorState.row_offset;
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
      const EditorRow &erow = editorState.rows[filerow];
      const std::string &line = erow.render;
      int linelen = line.size();
      int len;

      if (editorState.col_offset < linelen) {
        len = linelen - editorState.col_offset;
        if (len > editorState.screencols) {
          len = editorState.screencols;
        }
        buffer += line.substr(editorState.col_offset, len);
      }
    }

    buffer += "\x1b[K"; // clear the rest of the line

    if (y < editorState.screenrows - 1) {
      buffer += "\r\n"; // newline
    }
  }
}

void editorScroll() {
  // rendred cursor x
  int rx = 0;
  if (editorState.cursory < editorState.numrows) {
    const EditorRow &row = editorState.rows[editorState.cursory];
    // Convert cursor x from chars index to render index (tabs expanded)
    for (int j = 0;
         j < editorState.cursorx && j < static_cast<int>(row.chars.size());
         j++) {
      if (row.chars[j] == '\t') {
        // advance to the next tab stop
        rx += tabWidth - (rx % tabWidth);
      } else {
        rx++;
      }
    }
  }
  // Horizontall scrolling
  //
  // If the cursor moves left of the visible window,
  // adjust the horizontal scroll offset so the cursor's column
  // becomes the first visible column.
  if (rx < editorState.col_offset) {
    editorState.col_offset = rx;
  }

  // If the cursor moves right of the visible window,
  // scroll horizontally so the cursor appears in the last
  // visible column of the screen.
  if (rx >= editorState.col_offset + editorState.screencols) {
    editorState.col_offset = rx - editorState.screencols + 1;
  }

  // Vertical Scrolling
  //
  // If the cursor moves above the visible window,
  // adjust the vertical scroll offset so the cursor's row
  // becomes the first visible row.
  if (editorState.cursory < editorState.row_offset) {
    editorState.row_offset = editorState.cursory;
  }

  // If the cursor moves below the visible window,
  // scroll down so the cursor appears on the last visible row.
  if (editorState.cursory >= editorState.row_offset + editorState.screenrows) {
    editorState.row_offset = editorState.cursory - editorState.screenrows + 1;
  }
}
