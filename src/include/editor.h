#pragma once
#include <ctime>
#include <fstream>
#include <string>
#include <string_view>
#include <termios.h>
#include <vector>

// Get Ctrl + key code by keeping only the lower 5 bits
// Matches what keyboard sends for Ctrl+key (for example Ctrl+A sends 1)
#define CTRL_KEY(k) ((k) & 0x1f)

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

struct Editor {
  struct termios orig_termios;  // original terminal settings
  std::vector<EditorRow> rows;  // lines of the file
  int numrows = 0;              // total number of rows in the file
  int cursorx = 0;              // cursor column position in the current row
  int cursory = 0;              // cursor row position in the file
  int screenrows = 0;           // number of rows the terminal can display
  int screencols = 0;           // number of columns the terminal can display
  int row_offset = 0;           // vertical scroll offset
  int col_offset = 0;           // horizontal scroll offset
  std::string message;          // message for the user
  std::time_t message_time = 0; // timestamp when message was set
  std::string filename;         // current opened file name
  bool modified = false;        // true if the file has unsaved data
  std::string buffer;
  int tabWidth = 4;
  int quitTimes = 0;

  void init();
  int getWindowSize(int *rows, int *cols);
  void open(const std::string &filename);
  void updateRow(EditorRow &row);
  std::string expandTabs(std::string_view s);
  void setStatusMessage(std::string_view msg);
  void quit();
  void save();
  std::string rowsToString();
  int cxToRx(std::string_view chars, int cx); // CursorX to RenderX
  std::string prompt(const std::string &prompt,
                     void (*callback)(std::string, int));
  void find();
  static void findCallback(std::string query, int key);
};

namespace Terminal {
void enable();
void disable();
} // namespace Terminal

namespace Input {
int readKey();
void processKeyPress();
void handlePageKeys(int c);
} // namespace Input

namespace Output {
void refreshScreen();
void drawRows();
void scroll();
void drawStatusBar();
void drawMessageBar();
} // namespace Output

namespace Operations {
void insertChar(int c);
void deleteCharAt(int row, int col);
void moveCursor(int key);
} // namespace Operations

namespace Utils {
void clearScreen();
void die(std::string_view s);
template <typename T> void debugLog(T value) {
  std::ofstream f("tmp/debug.log", std::ios::app);
  if (f)
    f << value << '\n';
}
} // namespace Utils

extern Editor editor;
