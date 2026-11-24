# Known limitations and deviations

The following behaviours are intentional or currently unimplemented and are part of the implemented contract.

| Area | Limitation |
| --- | --- |
| Platform | The POSIX builds target Linux and macOS with a POSIX-style C library. The Windows build uses native process and console APIs. Platform-specific behaviour is noted where it matters. |
| Prompt settings | `show_mode`, `separator`, and `success` are parsed and stored but have no current rendering effect. `default_editor` is stored but no builtin currently uses it. |
| Shell expansion | `$@`, `$*`, positional parameters beyond `$0`, arrays, and associative shell arrays are not implemented. |
| Tilde expansion | `~user` is not resolved. |
| Globbing | Brace expansion, recursive `**`, and extglob are not implemented. |
| Aliases | Aliases are expanded for single-command input and not as stages of a multi-command pipeline. |
| Redirections | There are no heredocs, here-strings, process substitution, `|&`, or arbitrary descriptor redirections beyond `2>&N`. |
| Language features | The scripting language has no classes, comprehensions, generators, decorators, slicing, varargs, `with`, `assert`, `del`, `global`, or `is`. |
| Type annotations | Type annotations on function parameters, return values, and type fields are stored as metadata but are not enforced at runtime. |
| String literals | Single-quoted and double-quoted strings both interpolate. |
| Division | Integer division truncates toward zero. There is no floor-division operator. |
| Dictionaries | Dictionaries are unordered. Dictionary and function equality is identity equality. |
| Reference values | List and dictionary assignment aliases the object; there is no copy-on-write or deep copy. |
| String bytes | String length and `ord`/`chr` are byte-oriented. String indexing advances by UTF-8 code points only at index boundaries. |
| Line editor width | The line editor counts display width in code points, not terminal cells. |
| Block input | A line ending in `:` starts multiline collection in the interactive shell, but a trailing colon is not valid Blood syntax and is reported as a syntax error. |
| Windows `ls` | `ls` is a builtin only on Windows; Unix builds use the external command. |
