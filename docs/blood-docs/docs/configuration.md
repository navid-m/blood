# Configuration

## Location and lifecycle

The configuration file is located at `$XDG_CONFIG_HOME/blood/blood.ini` when `XDG_CONFIG_HOME` is set and non-empty. When that variable is unset or empty, the file is located at `$HOME/.config/blood/blood.ini`. On Windows, `$APPDATA/blood/blood.ini` is used when `APPDATA` is set, and the home-directory fallback applies otherwise.

The configuration directory and the directory containing the history file are created at interactive startup if they do not exist. The configuration file itself is not created automatically; `blood -init` writes a default file.

Configuration is loaded once at process startup. It is not reloaded while the shell is running, except through the `:rla` escape command or a `:cfg` update. A missing configuration file is not an error, and every setting has a default.

## File syntax

The file uses a sectioned INI syntax.

```ini
# comment
[section]
key = value
```

Leading and trailing whitespace is removed from keys and values. A line whose first non-whitespace byte is `#` or `;` is a comment. Inline comments are not recognised. Section headers have the form `[name]`. Keys before the first section are interpreted as belonging to the `prompt` section. A value may be enclosed in matching single or double quotation marks, and those marks are removed. Blank lines are ignored, and unknown keys are ignored. The `aliases` and `env` sections have special meaning. Sections other than those two do not restrict which keys they contain; a recognised key is applied regardless of the section in which it appears. A non-comment line without `=` produces a diagnostic on standard error and is skipped.

Boolean values are `true`, `yes`, `on`, and `1` for true, and `false`, `no`, `off`, and `0` for false. Boolean and integer keys are case sensitive.

A malformed line, an invalid boolean, or an invalid integer produces a diagnostic on standard error and the shell continues with the default or previously assigned value.

## Settings

### `[prompt]`

| Key | Type | Default and effect |
| --- | --- | --- |
| `theme` | string | `default`. Selects a prompt preset. Available presets are `default`, `plain`, `lambda`, `powerline`, `right`, `right_time`, `hacker`, `dots`, `compact`, `iconified`, `timestamped`, and `doubleline`. An unrecognised value behaves as `default`. |
| `show_username` | boolean | `true`. Includes the user name. |
| `show_hostname` | boolean | `true`. Includes the host name, normally with `@` between username and host. |
| `show_directory` | boolean | `true`. Includes the current directory. |
| `show_git` | boolean | `true`. Includes Git branch and dirty state when a Git repository is found. |
| `git_status` | boolean | Alias for `show_git`. |
| `show_time` | boolean | `false`. Includes the time. The default layout uses `HH:MM:SS`; presets that show time use `HH:MM`. |
| `show_exit_status` | boolean | `true`. Includes a failure marker and status when the previous status is non-zero. |
| `show_duration` | boolean | `true`. Includes the duration of the previous command when it is at least 500 milliseconds. |
| `show_jobs` | boolean | `true`. Includes the number of running background jobs when non-zero. |
| `show_mode` | boolean | `false`. Accepted but currently not rendered. |
| `two_line` | boolean | `true`. Selects the two-line box layout. Ignored when `theme` is `plain`. |
| `symbol` | string | `❯`. Sets both `prompt_symbol` and `prompt_char`. |
| `prompt_symbol` | string | `❯`. Sets both `prompt_symbol` and `prompt_char`. |
| `prompt_char` | string | `❯`. The character printed before the cursor. |
| `separator` | string | `─`. Accepted but currently not rendered. |
| `directory_style` | string | `full`. One of `full`, `short`, or `basename`. |

`full` shows the complete directory with the home directory abbreviated to `~`. `short` shows the final two path components. `basename` shows only the final component. An unrecognised value behaves as `full`.

### `[colors]`

| Key | Default | Use |
| --- | --- | --- |
| `primary` | 256-colour 124 | Username and prompt character. |
| `secondary` | 256-colour 88 | Host name. |
| `success` | `green` | Accepted for compatibility; currently not rendered. |
| `error` | `red` | Failure status. |
| `directory` | 256-colour 88 | Directory. |
| `git_clean` | `green` | Clean Git branch. |
| `git_dirty` | 256-colour 88 | Dirty Git branch and dirty marker. |
| `muted` | `bright_black` | Separators, box drawing, time, duration, and inline suggestions. |
| `warning` | `yellow` | Background job count. |
| `accent` | `bright_cyan` | Accent text in the login banner and accent glyphs in some presets. |

Colour values may be a name, a decimal 256-colour index, or a `#RRGGBB` value. The accepted names are `default`, `black`, `red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, `white`, `bright_black` (also `gray` and `grey`), `bright_red`, `bright_green`, `bright_yellow`, `bright_blue`, `bright_magenta`, `bright_cyan`, and `bright_white`. A decimal index selects a 256-colour foreground. A `#RRGGBB` value is rendered as a 24-bit foreground colour. An unrecognised colour produces no escape sequence. Styles such as bold or underline cannot be selected independently.

The colours written by `blood -init` differ from the built-in defaults: the generated file names `red` for the primary and secondary colours and `blue` for the directory. A configuration file that is created by `blood -init` therefore overrides the built-in 256-colour defaults with those named colours.

### `[history]`

| Key | Type | Default and effect |
| --- | --- | --- |
| `enabled` | boolean | `true`. Enables loading, recording, and saving history. |
| `file` | string | `~/.local/share/blood/history`. Leading `~` is expanded. On Windows the default is `$LOCALAPPDATA/blood/history` when that variable is set. |
| `history_file` | string | Alias for `file`. |
| `max_entries` | integer | `10000`. The maximum number of retained entries. |
| `history_max` | integer | Alias for `max_entries`. |
| `ignore_duplicates` | boolean | `true`. Suppresses a command identical to the immediately preceding command. |

### `[completion]`

| Key | Type | Default and effect |
| --- | --- | --- |
| `enabled` | boolean | `true`. Enables Tab completion. |
| `case_sensitive` | boolean | `false`. Controls prefix matching for completion and inline history suggestions. |
| `history_suggestions` | boolean | `false`. Enables inline history suggestions. |
| `autosuggestions` | boolean | Alias for `history_suggestions`. |
| `external` | boolean | `false`. Enables application completion through installed Bash completion definitions. |

### `[shell]`

| Key | Type | Default and effect |
| --- | --- | --- |
| `default_editor` | string | `nano`. Accepted and stored; no builtin currently uses it. |
| `editor` | string | Alias for `default_editor`. |
| `login_message` | boolean | `true`. Prints a one-line banner when an interactive session starts. |
| `login` | boolean | `false`. Initializes a login-shell environment as described in [Invocation](invocation.md). |
| `startup_file` | string | Empty. A script to execute before the interactive loop. Leading `~` is expanded. |

In addition, `~/.config/blood/init.blood` is executed before the interactive loop when it exists. If both `startup_file` and the default startup file exist, both are executed in that order. Startup scripts run after configuration and history loading.

### `.bloodrc`

Blood creates and runs `~/.config/blood/.bloodrc` for every interactive session. When `XDG_CONFIG_HOME` is set, the file lives beside `blood.ini` under that directory. The file is a normal Blood script and is created with a prelude stub when missing.

```blood
# Blood startup file.
# Run at the start of every interactive session.

# Aliases:
# alias ll='ls -l'
# alias gs='git status'

fn prelude() {
    # Session setup goes here.
}

prelude()
```

Aliases and functions defined in `.bloodrc` are available in the interactive shell. The file runs at the start of an interactive session, before `startup_file` and `init.blood`. `:rla` reloads the file.

### `[aliases]`

Each key is an alias name and each value is its replacement text.

```ini
[aliases]
ll = ls -l
gs = git status
```

Aliases defined here are installed before the interactive loop and before `startup_file` is executed. Blood also defines the convenience alias `dir` for `ls` before configuration aliases are applied.

### `[env]`

Each key is an environment variable name and each value is its value.

```ini
[env]
EDITOR = vim
PAGER = less
```

These variables are passed to the process environment before the shell initialises its variable table. An existing environment variable is replaced by the configured value.

## Default configuration file

`blood -init` writes the following content when the file does not exist.

```ini
# Blood shell configuration
# See `blood help config` for the available keys.
# Startup file: .bloodrc (aliases and prelude).
# Prompt themes: default, plain, lambda, powerline, right, right_time,
# hacker, dots, compact, iconified, timestamped, doubleline.

[prompt]
theme = default
show_username = true
show_hostname = true
show_directory = true
show_git = true
show_time = false
show_exit_status = true
show_duration = true
two_line = true

[colors]
primary = red
secondary = red
success = green
error = red
directory = blue
git_clean = green
git_dirty = yellow
muted = white

[history]
enabled = true
file = ~/.local/share/blood/history
max_entries = 10000
ignore_duplicates = true

[completion]
enabled = true
case_sensitive = false
history_suggestions = false
external = false

[shell]
default_editor = nano
login_message = true
login = false
```

The built-in defaults used when a key is absent are listed in [Appendix: default settings](appendix-defaults.md). The generated file is a starting point and does not list every key.
