#include "include/editor.h"
#include <cstdlib>
#include <termios.h>
#include <unistd.h>

namespace Terminal {

void disable() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &editor.orig_termios) == -1)
        Utils::die("tcsetattr");
}

void enable() {
    if (tcgetattr(STDIN_FILENO, &editor.orig_termios) == -1) {
        Utils::die("tcgetattr");
    }

    atexit(disable);

    struct termios raw = editor.orig_termios;
    /* Disable special handling of input bytes */
    raw.c_iflag &=
        ~(BRKINT | /* do not generate SIGINT on break condition */
          ICRNL |  /* do not translate carriage return (\r) to newline (\n) */
          INPCK |  /* disable input parity checking */
          ISTRIP | /* do not strip the 8th bit from input bytes */
          IXON);   /* disable software flow control (Ctrl-S / Ctrl-Q) */

    /* Disable all output post-processing */
    raw.c_oflag &= ~(OPOST); /* output bytes are written exactly as provided */

    /* Configure control flags */
    raw.c_cflag |= CS8; /* use 8-bit characters */

    /* Disable local (line discipline) features */
    raw.c_lflag &=
        ~(ECHO |   /* disable input echoing */
          ICANON | /* disable canonical (line-buffered) input mode */
          ISIG |   /* disable signal generation (Ctrl-C, Ctrl-Z, etc.) */
          IEXTEN); /* disable implementation-defined input processing */

    raw.c_cc[VMIN] =
        0; // minimum number of bytes of input needed before read() can return
    raw.c_cc[VTIME] = 1; // maximum amount of time to wait before read() returns
                          // (1 == 100ms)

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        Utils::die("tcsetattr");
    }
}

}
