#pragma once
#include <string>
#include <termios.h>
#include <vector>

#define CTRL_KEY(k) ((k) & 0x1f)

enum editorKey {
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

struct EditorConfig {
  int cursorx;
  int cursory;
  int screenrows;
  int screencols;
  int numrows;
  int row_offest;
  int col_offset;
  struct termios orig_termios;
  std::vector<std::string> rows;
};

extern struct EditorConfig editorConfig;
extern std::string buffer;

int getWindowSize(int *rows, int *cols);
void initEditor();
void editorOpen(const std::string &filename);
