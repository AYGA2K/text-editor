#include "include/operations.h"
#include "include/editor.h"

void editorInsertChar(int c) {
  int row = editorConfig.cursory;
  int col = editorConfig.cursorx;

  // Create missing rows if needed
  if (row >= editorConfig.numrows) {
    while (editorConfig.numrows <= row) {
      editorConfig.rows.push_back("");
      editorConfig.numrows++;
    }
  }

  // Fill row with spaces if cursor is past the end
  if (col >= static_cast<int>(editorConfig.rows[row].size())) {
    editorConfig.rows[row].resize(col, ' ');
  }

  editorConfig.rows[row].insert(col, 1, static_cast<char>(c));
  editorConfig.cursorx++;
}

void deleteCharAt(int row, int col) {
  if (editorConfig.rows[row][col]) {
    editorConfig.rows[row].erase(col, 1);
  }
}

void moveCursor(int key) {
  switch (key) {
  case ARROW_LEFT:
    if (editorConfig.cursorx != 0) {
      editorConfig.cursorx--;
    }
    break;
  case ARROW_RIGHT:
    if (editorConfig.cursory < editorConfig.numrows &&
        editorConfig.cursorx <
            (int)editorConfig.rows[editorConfig.cursory].size()) {
      editorConfig.cursorx++;
    }
    break;
  case ARROW_UP:
    if (editorConfig.cursory != 0) {
      editorConfig.cursory--;
    }
    break;
  case ARROW_DOWN:
    if (editorConfig.cursory < editorConfig.numrows) {
      editorConfig.cursory++;
    }
    break;
  }
}
