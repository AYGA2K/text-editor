#include "include/operations.h"
#include "include/editor.h"
#include <cstddef>

void editorInsertChar(int c) {
  int row = editorState.cursory;
  int col = editorState.cursorx;

  // Create missing rows if needed
  if (row >= editorState.numrows) {
    while (editorState.numrows <= row) {
      EditorRow newRow;
      editorState.rows.push_back(newRow);
      editorState.numrows++;
    }
  }

  // Fill row with spaces if cursor is past the end
  if (col >= static_cast<int>(editorState.rows[row].chars.size())) {
    editorState.rows[row].chars.resize(col, ' ');
  }

  editorState.rows[row].chars.insert(col, 1, static_cast<char>(c));
  editorUpdateRow(editorState.rows[row]);
  editorState.cursorx++;
}

void deleteCharAt(int row, int col) {
  if (row < 0 || row >= editorState.numrows)
    return;
  if (col < 0 || col >= static_cast<int>(editorState.rows[row].chars.size()))
    return;

  editorState.rows[row].chars.erase(col, 1);
  editorUpdateRow(editorState.rows[row]);
}

void moveCursor(int key) {
  EditorRow *row = (editorState.cursory >= editorState.numrows)
                       ? NULL
                       : &editorState.rows[editorState.cursory];
  switch (key) {
  case ARROW_LEFT:
    if (editorState.cursorx != 0) {
      editorState.cursorx--;
    }
    break;
  case ARROW_RIGHT:
    if (row && editorState.cursorx < static_cast<int>(row->chars.size())) {
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

  row = (editorState.cursory >= editorState.numrows)
            ? NULL
            : &editorState.rows[editorState.cursory];
  int rowLen = row ? static_cast<int>(row->chars.size()) : 0;
  if (editorState.cursorx > rowLen) {
    editorState.cursorx = rowLen;
  }
}
