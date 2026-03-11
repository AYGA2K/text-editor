#include "include/operations.h"
#include "include/editor.h"

void editorInsertChar(int c) {
  int row = editorState.cursory;
  int col = editorState.cursorx;

  // Create missing rows if needed
  if (row >= editorState.numrows) {
    while (editorState.numrows <= row) {
      editorState.rows.push_back("");
      editorState.numrows++;
    }
  }

  // Fill row with spaces if cursor is past the end
  if (col >= static_cast<int>(editorState.rows[row].size())) {
    editorState.rows[row].resize(col, ' ');
  }

  editorState.rows[row].insert(col, 1, static_cast<char>(c));
  editorState.cursorx++;
}

void deleteCharAt(int row, int col) {
  if (editorState.rows[row][col]) {
    editorState.rows[row].erase(col, 1);
  }
}

void moveCursor(int key) {
  switch (key) {
  case ARROW_LEFT:
    if (editorState.cursorx != 0) {
      editorState.cursorx--;
    }
    break;
  case ARROW_RIGHT:
    if (editorState.cursory < editorState.numrows &&
        editorState.cursorx <
            (int)editorState.rows[editorState.cursory].size()) {
      editorState.cursorx++;
    }
    break;
  case ARROW_UP:
    if (editorState.cursory != 0) {
      editorState.cursory--;
    }
    break;
  case ARROW_DOWN:
    if (editorState.cursory < editorState.numrows) {
      editorState.cursory++;
    }
    break;
  }
}
