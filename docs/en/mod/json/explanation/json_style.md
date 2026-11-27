> **Unstable documentation.**

# JSON Style Guide

Like in [C++ Code Style](../../../dev/explanation/code_style.md), the JSON styling policy is to
update JSON as it is added or edited, and in relatively small chunks otherwise in order to prevent
undue disruption to development.

## Why do we have a homegrown JSON formatter?

DDA wrote their own JSON parser. It lives in `tools/format/format.cpp` and it leverages
`src/json.cpp`to parse and emit JSON.

This isn't optimal solution as it makes using existing JSON formatters impossible,
but [last attempt](https://github.com/cataclysmbn/Cataclysm-BN/pull/3118) proved that the
disadvantages outweighed possible benefits.

## JSON Example

This example outlines most of the styling features:

```json
[
  {
    "type": "foo",
    "id": "example",
    "short_array": [1, 2, 3, 4, 5],
    "short_object": {
      "item_a": "a",
      "item_b": "b"
    },
    "long_array": [
      "a really long string to illustrate line wrapping, ",
      "which occurs if the line is longer than 120 characters"
    ],
    "nested_array": [
      [
        ["item1", "value1"],
        ["item2", "value2"],
        ["item3", "value3"],
        ["item4", "value4"],
        ["item5", "value5"],
        ["item6", "value6"]
      ]
    ]
  }
]
```

Indention is two spaces. All JSON delimiters except comma and colon are surrounded by whitespace
(either a space or a newline). Comma and colon are followed by whitespace. Object entries are always
newline-separated. Array entries are newline-separated if the resulting array would exceed 120
characters otherwise (including indention). Line breaks occur after open brackets, close brackets,
or entries.

## Formatting tool

The formatting tool can be invoked through the CMake `style-json` target, directly as
`tools/format/json_formatter.cgi`, or via cgi at http://dev.narc.ro/cataclysm/format.html

### For Single file

To style one JSON file, run `json_formatter path/to/file.json`. This styles that file. If it prints
"needs linting", the file was not styled and should be reviewed. You can also drag a JSON file onto
the `json_formatter` icon.

### On *nix

Run `build-scripts/fmt.sh json` in the repository root.
