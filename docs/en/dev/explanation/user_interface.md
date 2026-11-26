# User Interface

Cataclysm's browser build uses a tiles implementation of the curses-style user interface.
Window management is achieved by `ui_adaptor`, which requires a resizing callback and a
redrawing callback for each UI to handle resizing and redrawing. Details on how to use `ui_adaptor`
can be found within `ui_manager.h`.

Some good examples of the usage of `ui_adaptor` can be found within the following files:

- `query_popup` and `static_popup` in `popup.h/cpp`
- `Messages::dialog` in `messages.cpp`
