# context

![demo](img/img.png)

context is a mode-based console text editor.


## Features

- syntax highlighting
- command-based config file format
- Windows support
- UTF-8 support
- common movement keys are accessible on the home row
  by pressing Alt and/or Ctrl
- style configuration
- remappable keybinds


## Requirements

- Python 3
- ncurses (Linux)


## Help

Press F1 to bring up the help page and press Ctrl-q to quit.

## Installation

### Linux

```
./builder.py release
sudo ./builder.py install
```

This will install context at `/usr/local/bin/ctx`.

### Windows

```
python builder.py windows release
```

MSYS2 or an equivalent build system is needed
to build context on Windows.
This command will build a portable executable of
context in `bin/ctx`.

## Configuration

context will look for a config file at `~/.ctxcfg`.
The help page explains what can be configured.
