# Bash translation

`-translate-sh` converts the common bash scripting constructs into their Blood equivalents while keeping the result ordinary, editable Blood source. Translation is deliberately conservative: a construct that has no faithful Blood equivalent is retained as a `shell` statement or emitted as a comment.

The following constructs are translated.

| Bash construct | Blood result |
| --- | --- |
| Variable assignment (`NAME=VALUE`, `NAME+=VALUE`), arrays, `local`, `declare`, `readonly`, and `export` | A Blood assignment. Assigned names are seeded from the environment before the script body runs and are mirrored into the shell variable table so later commands can still use `$NAME`. Array variables become Blood lists. |
| `echo` | `say` when the arguments can be expressed as Blood values. |
| `if`, `elif`, `else`, `while`, `until`, `for NAME in LIST`, C-style `for ((...))`, `case`, and function declarations | Brace-delimited Blood statements. `case` clauses become an `if`/`elif` chain. |
| `[ ... ]`, `[[ ... ]]`, and `test ...` | Blood expressions. String tests, numeric tests, boolean operators, and common file tests are supported. File tests use the platform `test` utility through `shell`. |
| `$(...)` and backtick command substitution | `sh(...)`. |
| `$((...))` arithmetic | A Blood arithmetic expression. |
| `$NAME`, `${NAME}`, `${NAME:-default}`, `${#NAME}`, positional parameters, `$?`, `$#`, and `$@` | Blood variables and the standard `args`, `status`, and `__file__` values. |
| `read -r NAME` | `NAME = std.input()`. Later commands are wrapped in `sh(...)` so arbitrary command lines, pipelines, and redirections keep working even when the script parser would otherwise read them as expressions. |

The translator does not implement heredocs, process substitution, traps, bash `set` options, `BASH_SOURCE`, associative arrays, or arbitrary parameter expansion modifiers. These are preserved as comments or ordinary shell statements and may need manual adjustment. `shift` inside a function, declarations with multiple targets, and declarations that cannot be expressed as simple assignments are also preserved as comments.

Translation produces an ordinary script. The output is not guaranteed to be idiomatic Blood, and a translated construct may still require review when it depends on bash-specific runtime state.
