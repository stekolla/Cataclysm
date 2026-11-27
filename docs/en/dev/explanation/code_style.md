# C++ Code Style Guide

All C++ code in the project is styled. Run `build-scripts/fmt.sh` before pushing, or `build-scripts/fmt.sh cpp` for C++-only changes. The helper uses astyle only for top-level `src/*.cpp` and `src/*.h`; most other C++ files use clang-format. Formatter fixtures are left unchanged.

Where astyle is still used, use version 3.1. Version 3.0.1 will disagree in only a few places, while Version 3.6.6 will disagree in nearly every file.

Blocks of code can be passed through astyle to ensure that their formatting is correct:

```sh
astyle --style=1tbs --attach-inlines --indent=spaces=4 --align-pointer=name --max-code-length=100 --break-after-logical --indent-classes --indent-switches --indent-preproc-define --indent-col1-comments --min-conditional-indent=0 --pad-oper --add-braces --convet-tabs --unpad-paren --pad-paren-in --keep-one-line-blocks
```

These options are mirrored in `.astylerc` and `doc/CODE_STYLE.txt`.

For example, from `vi`, set marks a and b around the block, then:

```sh
:'a,'b ! astyle --style=1tbs --attach-inlines --indent=spaces=4 --align-pointer=name --max-code-length=100 --break-after-logical --indent-classes --indent-switches --indent-preproc-define --indent-col1-comments --min-conditional-indent=0 --pad-oper --add-braces --convet-tabs --unpad-paren --pad-paren-in --keep-one-line-blocks
```

See [DEVELOPER_TOOLING.md](../reference/tooling.md) for other environments.

## Code Example

Here's an example that illustrates the most common points of style:

```cpp
auto foo( int arg1, int *arg2 ) -> int
{
    if( arg1 < 5 ) {
        switch( *arg2 ) {
            case 0:
                return arg1 + 5;
                break;
            case 1:
                return arg1 + 7;
                break;
            default:
                return 0;
                break;
        }
    } else if( arg1 > 17 ) {
        int i = 0;
        while( i < arg1 ) {
            printf( _( "Really long message that's pointless except for the number %d and for its "
                       "length as it's illustrative of how to break strings properly.\n" ), i );
        }
    }
    return 0;
}
```

## Code Guidelines

These are less generic guidelines and more pain points we've stumbled across over time.

### Current C++ conventions

- Use C++23, `auto` for local variables (`const auto` for immutable values), and trailing return
  types for functions and function type aliases, such as `std::function<auto( int ) -> bool>`.
- Use single-line functions and lambdas where possible. Prefer `auto` lambda parameters and
  direct function or member references when they can replace a lambda.
- In astyle files (top-level `src/*.cpp` and `src/*.h`), append `// *NOPAD*` to reference and
  pointer returns and to `<=>` declarations. Do not add this workaround in clang-format files.
- Use `snake_case`, triple-slash documentation comments, and `<=>` for comparisons. Keep unused
  parameter names as comments, for example `bool /*is_avatar*/`.
- Use designated initializers. For functions with more than three parameters, use an options
  struct and pass `{ .field = value }` without repeating the type when it can be inferred.
- Prefer a named struct over returning a pair or tuple. Use `std::optional` for a possibly absent
  value and `std::expected` for a result that may fail.
- Use anonymous namespaces instead of `static` for file-local declarations.
- Use `std::ranges`, `std::views`, `ranges::to`, and helpers from `cata_algo.h` for collection
  processing, with function-local `using namespace std::views;` and
  `namespace ranges = std::ranges;`. Avoid manual iterator increments unless mutation requires
  them. Keep plain loops when index access, early exits, mutation, or nesting is clearer.
- Prefer a new header with pure functions over modifying a header with more than ten usages.
- Put new C++ files in subdirectories so clang-format covers them. After moving C++ files into
  a subdirectory, run `scripts/format-trailing-return.sh <paths>` and
  `TIDY_CHECK=modernize-use-auto scripts/format-trailing-return.sh <paths>` on those files.
- Wrap user-facing C++ strings in `_( "text" )` for translation.

### Prefer Immutable Values

- Prefer immutable values and declare variables with `const`. Less moving parts mean more predictable code flow.

### Use `int` or `int64_t`

- Prefer `int`.
  - `long` in particular is problematic since it is _not_ a larger type than int on some platforms
    we support.
  - Using integral value larger than 32 bits should be avoided. Use `int64_t` if it's really necessary.
- `uint` is also a problem, it has poor behavior when overflowing and should be avoided for
  general purpose programming.
  - If you need binary data, `unsigned int` or `unsigned char` are fine, but you should probably
    use a `std::bitset` instead.
- `float` is to be avoided, but has valid uses.

### Use Auto Keyword

Examples of [`auto`](https://learn.microsoft.com/en-us/cpp/cpp/auto-cpp?view=msvc-170):

#### Trailing Return Types

- Use [trailing return types](https://en.wikipedia.org/wiki/Trailing_return_type) in function declarations. Long return types can obscure function names.
  ```cpp
  class Bar;
  auto foo( int a ) -> int
  {
      const auto &bar = some_function();

      return is_bar_ok( bar ) ? 42 : 404;
  }
  ```

#### Decltype Style Generic Functions

- Use for `decltype` style generic functions
  ```diff
  template<typename A, typename B>
  - decltype(std::declval<A&>() * std::declval<B&>()) multiply(A a, B b)
  + auto multiply( A a, B b ) -> decltype( a * b )
  {
      return a*b;
  }
  ```

#### Aliasing for Long Iterator Declarations

- Aliasing for long iterator declarations
  ```diff
    std::map<int, std::map<std::string, some_long_typename>> some_map;

  - std::map<int, std::map<std::string, some_long_typename>>::iterator iter = some_map.begin();
  + auto iter = some_map.begin();
  ```

#### Lambda Declarations

- Required for Lambda declarations
  ```cpp
  auto two_times = []( int a ) { return a * 2; };
  ```

#### Editor Type Hinting

- Doesn't otherwise sacrifice readability for expedience. Options for inlay type hinting are available in popular code editor such as [vscode](https://github.com/clangd/vscode-clangd).

### Avoid `using namespace`

- Avoid importing standard namespaces at global scope. The function-local `std::views`
  convention above keeps the import scoped to the pipeline.

### Avoid Member Methods

- Avoid adding new member methods to classes unless required.
  ```diff
  // this function does not access non-public data members or member methods in the class, and thus can be made a free function
  - std::string Character::profession_description() const
  - {
  -     return prof->description( male );
  - }
  + auto profession_description( const Character &c ) -> std::string
  + {
  +     return c.prof->description( c.male );
  + }
  ```
