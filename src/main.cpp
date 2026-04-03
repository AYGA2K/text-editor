#include "include/editor.h"

int main(int argc, char *argv[]) {
    Terminal::enable();
    editor.init();
    if (argc >= 2) {
        editor.open(argv[1]);
    }
    editor.setStatusMessage(
        "HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");
    while (1) {
        Output::refreshScreen();
        Input::processKeyPress();
    }
    return 0;
}
