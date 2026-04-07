#include "include/editor.h"
#include <cstddef>

namespace Operations {

void insertChar(int c) {
  int row = editor.cursory;
  int col = editor.cursorx;

  // Create missing rows if needed
  if (row >= editor.numrows) {
    while (editor.numrows <= row) {
      EditorRow newRow;
      editor.rows.push_back(newRow);
      editor.numrows++;
    }
  }

  // Fill row with spaces if cursor is past the end
  if (col >= static_cast<int>(editor.rows[row].chars.size())) {
    editor.rows[row].chars.resize(col, ' ');
  }

  editor.rows[row].chars.insert(col, 1, static_cast<char>(c));
  editor.updateRow(editor.rows[row]);
  editor.cursorx++;
  editor.modified = true;
}

void deleteCharAt(int row, int col) {
  if (row < 0 || row >= editor.numrows)
    return;
  if (col < 0 || col >= static_cast<int>(editor.rows[row].chars.size()))
    return;

  editor.rows[row].chars.erase(col, 1);
  editor.updateRow(editor.rows[row]);
}

void moveCursor(int key) {
  EditorRow *row =
      (editor.cursory >= editor.numrows) ? NULL : &editor.rows[editor.cursory];
  switch (key) {
  case ARROW_LEFT:
    if (editor.cursorx != 0) {
      editor.cursorx--;
    }
    break;
  case ARROW_RIGHT:
    if (row && editor.cursorx < static_cast<int>(row->chars.size())) {
      editor.cursorx++;
    }
    break;
  case ARROW_UP:
    if (editor.cursory != 0) {
      editor.cursory--;
    }
    break;
  case ARROW_DOWN:
    if (editor.cursory < editor.numrows - 1) {
      editor.cursory++;
    }
    break;
  }

  row =
      (editor.cursory >= editor.numrows) ? NULL : &editor.rows[editor.cursory];
  int rowLen = row ? static_cast<int>(row->chars.size()) : 0;
  if (editor.cursorx > rowLen) {
    editor.cursorx = rowLen;
  }
}

void handlePageKeys(int c) {
  if (c == PAGE_UP) {
    editor.cursory = editor.row_offset;
  } else if (c == PAGE_DOWN) {
    editor.cursory = editor.row_offset + editor.screenrows - 1;
    if (editor.cursory > editor.numrows) {
      editor.cursory = editor.numrows;
    }
  }
  int times = editor.screenrows;
  while (times > 0) {
    moveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
    times--;
  }
}

void handleBackspace() {
  const int previousRowIndex =
      editor.cursory - 1 >= 0 ? editor.cursory - 1 : -1;
  if ((editor.cursorx == 0) && (previousRowIndex >= 0)) {
    const EditorRow currentRow = editor.rows[editor.cursory];
    editor.rows[previousRowIndex].chars += currentRow.chars;
    editor.rows[previousRowIndex].render += currentRow.render;
    editor.rows.erase(editor.rows.begin() + editor.cursory);
    editor.cursory--;
    editor.cursorx = editor.rows[previousRowIndex].chars.size();
  }
  deleteCharAt(editor.cursory, editor.cursorx - 1);
  if (editor.cursorx > 0) {
    editor.cursorx--;
  }
}

void handleEnter() {
  const int currentRowIndex = editor.cursory;
  if (editor.cursorx == 0 ||
      editor.cursorx >=
          static_cast<int>(editor.rows[currentRowIndex].chars.size()) - 1) {
    EditorRow row = {};
    if (currentRowIndex + 1 < static_cast<int>(editor.rows.size())) {
      editor.rows.insert(editor.rows.begin() + currentRowIndex + 1, row);
    } else {
      editor.rows.push_back(row);
    }
  } else {
    const EditorRow currentRow = editor.rows[currentRowIndex];

    const std::string cursorEndRowChars = currentRow.chars.substr(
        editor.cursorx, currentRow.chars.size() - editor.cursorx);

    int rx = editor.cxToRx(currentRow.chars, editor.cursorx);

    const std::string cursorEndRowRender = currentRow.render.substr(rx);

    editor.rows[currentRowIndex].chars =
        currentRow.chars.substr(0, editor.cursorx);
    editor.rows[currentRowIndex].render = currentRow.render.substr(0, rx);

    EditorRow newRow = {};
    newRow.chars = cursorEndRowChars;
    newRow.render = cursorEndRowRender;
    editor.rows.insert(editor.rows.begin() + currentRowIndex + 1, newRow);
  }
  editor.numrows++;
  editor.cursorx = 0;
  editor.cursory++;
}
} // namespace Operations
