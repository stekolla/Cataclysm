# Compatibility with screen readers

Terminal screen readers use cursor position to identify information such as the selected item in a
list. The browser build renders to a canvas rather than a terminal; its screen-reader compatibility
has not been verified by this guide.

The recommended way to place the cursor is to use `ui_adaptor`. This ensures the desired cursor
position is preserved when subsequent output code changes the cursor position. You can call
`ui_adaptor::set_cursor` and similar methods at any position in a redrawing callback, and the last
cursor position of the topmost UI set via the call will be used as the final cursor position. You
can also call `ui_adaptor::disable_cursor` to prevent a UI's cursor from being used as the final
cursor position.
