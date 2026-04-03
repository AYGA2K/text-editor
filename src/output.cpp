#include "include/editor.h"
#include <cstddef>
#include <ctime>
#include <sstream>
#include <string>
#include <unistd.h>

namespace Output {

void refreshScreen() {
  scroll();
  editor.buffer.append(
      "\x1b[H"); // reposition the cursor back up at the top-left corner

  drawRows();
  drawStatusBar();
  drawMessageBar();

  int rx = 0;
  if (editor.cursory < editor.numrows) {
    const EditorRow &row = editor.rows[editor.cursory];
    for (int j = 0;
         j < editor.cursorx && j < static_cast<int>(row.chars.size()); j++) {
      if (row.chars[j] == '\t') {
        rx += (editor.tabWidth - 1) - (rx % editor.tabWidth) + 1;
      } else {
        rx++;
      }
    }
  }

  std::ostringstream oss;
  oss << "\x1b[" << editor.cursory - editor.row_offset + 1 << ";"
      << rx - editor.col_offset + 1 << "H"; // reposition the cursor
  editor.buffer.append(oss.str());

  size_t n = write(STDOUT_FILENO, editor.buffer.data(), editor.buffer.size());
  if (n == -1) {
    Utils::die("Error writing to stdout");
  }

  editor.buffer.clear();
}

void drawRows() {
  for (int y = 0; y < editor.screenrows; y++) {
    int filerow = y + editor.row_offset;
    if (filerow >= editor.numrows) {
      if (editor.numrows == 0 && y == editor.screenrows / 3) {
        std::string welcome = "The editor";
        int welcomelen = static_cast<int>(welcome.size());
        if (welcomelen > editor.screencols)
          welcomelen = editor.screencols;

        int padding = (editor.screencols - welcomelen) / 2;
        if (padding > 0) {
          editor.buffer.append("~");
          padding--;
        }
        editor.buffer.append(std::string(padding, ' '));
        editor.buffer.append(welcome.substr(0, welcomelen));
      } else {
        editor.buffer.append("~");
      }
    } else {
      const EditorRow &erow = editor.rows[filerow];
      const std::string &line = erow.render;
      int linelen = line.size();
      int len;

      if (editor.col_offset < linelen) {
        len = linelen - editor.col_offset;
        if (len > editor.screencols) {
          len = editor.screencols;
        }
        editor.buffer.append(line.substr(editor.col_offset, len));
      }
    }

    editor.buffer.append("\x1b[K"); // clear the rest of the line
    editor.buffer.append("\r\n");   // newline
  }
}

void scroll() {
  // rendred cursor x
  int rx = 0;
  if (editor.cursory < editor.numrows) {
    const EditorRow &row = editor.rows[editor.cursory];
    // Convert cursor x from chars index to render index (tabs expanded)
    for (int j = 0;
         j < editor.cursorx && j < static_cast<int>(row.chars.size()); j++) {
      if (row.chars[j] == '\t') {
        // advance to the next tab stop
        rx += editor.tabWidth - (rx % editor.tabWidth);
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
  if (rx < editor.col_offset) {
    editor.col_offset = rx;
  }

  // If the cursor moves right of the visible window,
  // scroll horizontally so the cursor appears in the last
  // visible column of the screen.
  if (rx >= editor.col_offset + editor.screencols) {
    editor.col_offset = rx - editor.screencols + 1;
  }

  // Vertical Scrolling
  //
  // If the cursor moves above the visible window,
  // adjust the vertical scroll offset so the cursor's row
  // becomes the first visible row.
  if (editor.cursory < editor.row_offset) {
    editor.row_offset = editor.cursory;
  }

  // If the cursor moves below the visible window,
  // scroll down so the cursor appears on the last visible row.
  if (editor.cursory >= editor.row_offset + editor.screenrows) {
    editor.row_offset = editor.cursory - editor.screenrows + 1;
  }
}

void drawStatusBar() {
  // Set background and foreground colors
  // Format: \x1b[<fg>;<bg>m
  editor.buffer.append("\x1b[38;5;250;48;5;238m");
  std::string barContent;
  if (!editor.filename.empty()) {
    barContent.append(editor.filename);
  } else {
    barContent.append("[No name]");
  }

  barContent.append(" " + std::to_string(editor.numrows) + " lines");

  int remaining = editor.screencols - barContent.size();
  if (remaining > 0) {
    // Fill the status bar with spaces
    barContent.append(std::string(remaining, ' '));
  }
  editor.buffer.append(barContent);

  // Reset terminal formatting
  editor.buffer.append("\x1b[m");
  editor.buffer.append("\r\n");
}

void drawMessageBar() {
  editor.buffer.append("\x1b[K"); // clear the message bar
  const std::time_t now = std::time(NULL);
  if (now - editor.message_time < 5) {
    editor.buffer.append(editor.message);
  }
}
} // namespace Output
