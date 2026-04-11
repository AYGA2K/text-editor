# text-editor

A small terminal text editor in C++. It uses raw mode, draws to the terminal, and edits a file.

<video src="demo.mp4" controls></video>

## Build

Needs a C++ compiler (the makefile uses `g++`).

```bash
make
```

The binary is `build/main`.

## Run

```bash
./build/main
```

Optional: pass a file path to open it.

```bash
./build/main path/to/file.txt
```

## Keys

- **Ctrl-S** — save
- **Ctrl-Q** — quit (may ask to confirm if there are unsaved changes)
- **Ctrl-F** — find

The status line at the bottom shows the same hints when you start.

## Clean

```bash
make clean
```
