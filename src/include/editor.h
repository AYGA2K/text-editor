#pragma once
#include <ctime>
#include <string>
#include <termios.h>
#include <vector>

#define CTRL_KEY(k) ((k) & 0x1f)

enum Mode { NORMAL, INSERT, VISUAL, COMMAND };

enum EditorKey {
  TAB = 9,
  ENTER = 13,
  ESCAPE = 27,
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  PAGE_UP,
  PAGE_DOWN,
  HOME_KEY,
  END_KEY,
  DEL_KEY,
};

struct EditorRow {
  std::string chars;  // raw characters (may contain '\t')
  std::string render; // rendered characters (tabs expanded to spaces)
};

struct EditorState {
  struct termios orig_termios; // original terminal settings
  std::vector<EditorRow> rows; // lines of the file
  int numrows;                 // total number of rows in the file
  int cursorx;                 // cursor column position in the current row
  int cursory;                 // cursor row position in the file
  int screenrows;              // number of rows the terminal can display
  int screencols;              // number of columns the terminal can display
  int row_offset;              // vertical scroll offset
  int col_offset;              // horizontal scroll offset
  std::string message;         // message for the user
  std::time_t message_time;    // timestamp when message was set
  std::string filename;        // current opened file name
  bool modified;               // true if the file has unsaved data
};

extern struct EditorState editorState;
extern std::string buffer;
extern int tabWidth;

int getWindowSize(int *rows, int *cols);
void initEditor();
void editorOpen(const std::string &filename);
void editorUpdateRow(EditorRow &row);
void editorQuit();
void editorSetStatusMessage(std::string msg);
void editorSave();
int cxToRx(const std::string_view chars, int cx); // CursorX to RenderX
