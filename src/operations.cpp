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
    if (col < 0 ||
        col >= static_cast<int>(editor.rows[row].chars.size()))
        return;

    editor.rows[row].chars.erase(col, 1);
    editor.updateRow(editor.rows[row]);
}

void moveCursor(int key) {
    EditorRow *row = (editor.cursory >= editor.numrows)
                         ? NULL
                         : &editor.rows[editor.cursory];
    switch (key) {
    case ARROW_LEFT:
        if (editor.cursorx != 0) {
            editor.cursorx--;
        }
        break;
    case ARROW_RIGHT:
        if (row &&
            editor.cursorx < static_cast<int>(row->chars.size())) {
            editor.cursorx++;
        }
        break;
    case ARROW_UP:
        if (editor.cursory != 0) {
            editor.cursory--;
        }
        break;
    case ARROW_DOWN:
        if (editor.cursory < editor.numrows) {
            editor.cursory++;
        }
        break;
    }

    row = (editor.cursory >= editor.numrows)
              ? NULL
              : &editor.rows[editor.cursory];
    int rowLen = row ? static_cast<int>(row->chars.size()) : 0;
    if (editor.cursorx > rowLen) {
        editor.cursorx = rowLen;
    }
}

}
