# Interactive terminal

## Terminal selection

The interactive line editor is enabled only when the platform terminal query succeeds on standard input. If standard input is not a terminal, Blood reads lines without prompt editing and without a prompt.

The terminal size is queried with the platform window-size interface; on POSIX systems this is `TIOCGWINSZ`. The fallback size is 80 columns by 24 rows.

## Raw mode

When the line editor is entered, Blood saves the current terminal settings and switches to a raw-like mode. `ECHO`, `ICANON`, `IEXTEN`, and `ISIG` are cleared from the local flags. `IXON` and `ICRNL` are cleared from the input flags. `VMIN` is 1 and `VTIME` is 0.

The original settings are restored before any command is executed and when the shell exits. This makes Ctrl-C and Ctrl-Z work normally for external programs.

The editor assumes an ANSI-compatible terminal and emits ANSI CSI sequences for clearing, cursor movement, scrolling, and menu drawing. No terminfo database is used.

## Rendering model

The prompt text is computed once per command line, not once per keystroke. Each keystroke redraws the prompt, the current line, and any open completion menu in a single buffered write. Long lines are horizontally scrolled so that the cursor remains visible. Prompts that span several rows are supported.

Newline bytes stored in a recalled history entry are displayed as `⏎` in the line editor. Cursor movement traverses UTF-8 code points. Display width is counted in code points, not in terminal cells, so wide East Asian characters and combining marks are not width-corrected.

Inline history suggestions may be drawn after the cursor when `[completion] history_suggestions` is enabled. The suggestion is the newest history entry that extends the current line and contains no line break. It is shown in the muted colour and is accepted with the Right or End key when the cursor is at the end of the line.

For terminal integration, the editor emits OSC 133 semantic prompt and command markers. When the `TERM_PROGRAM` environment variable is `vscode`, it emits OSC 633 markers instead and announces rich command detection and the current working directory.

The completion menu is drawn as a bordered rectangle below the prompt. Its height is capped at ten rows, and it never fills the terminal. The cursor is repositioned to the insertion point after each redraw.

## Key bindings

| Key | Action |
| --- | --- |
| Printable character | Insert at the cursor. |
| Enter | Accept the current line. While a history search is active, accept the matched line. |
| Ctrl-A | Move to the beginning of the line. |
| Ctrl-E | Move to the end of the line. |
| Ctrl-B | Move one code point left. |
| Ctrl-F | Move one code point right; at the end of the line, accept an inline history suggestion. |
| Ctrl-K | Delete from the cursor to the end of the line. |
| Ctrl-U | Delete from the beginning of the line to the cursor. |
| Ctrl-W | Delete the previous word. |
| Ctrl-L | Clear the screen and redraw. |
| Ctrl-P | Recall the previous history entry. |
| Ctrl-N | Recall the next history entry. |
| Ctrl-R | Start or advance reverse incremental history search. |
| Ctrl-C | Cancel the current line. Prints `^C` and returns to a fresh prompt. |
| Ctrl-D | On an empty line, end the session. Otherwise delete at the cursor. |
| Ctrl-Z | Ignored by the editor; the signal is ignored by the shell. |
| Backspace | Delete the previous code point. |
| Delete | Delete the code point at the cursor. |
| Left / Right | Move by one code point. At the end of the line, Right accepts an inline history suggestion. |
| Up / Down | Navigate history, or the completion menu when it is open. |
| Home / End | Move to the beginning or end of the line. End also accepts an inline history suggestion. |
| Tab | Complete at the cursor, or advance the completion menu. |
| Shift-Tab | Move backwards through the completion menu. |
| Esc | Close the completion menu. A lone Esc is ignored. |
| Alt-b / Alt-f | Move one word left or right. |
| Alt-Left / Alt-Right, Ctrl-Left / Ctrl-Right | Move one word left or right. |

Unrecognised control bytes are ignored.

## Completion menu

Completion is opened with Tab. If there is one candidate, it is inserted immediately. If there are multiple candidates and they share a longer common prefix than the current word, that prefix is inserted and the menu is opened. Otherwise the menu is opened without changing the line.

While the menu is open, Tab selects the next candidate and previews it in the line, Shift-Tab selects the previous candidate and previews it, and Enter accepts the line as previewed. Any ordinary editing key closes the menu and acts on the previewed line. Esc closes the menu and restores the line to the state it had when the menu opened; a lone Esc with no menu open is ignored.

The menu is drawn as a bordered rectangle with a title that identifies the candidate kind and the number of candidates. Candidate kinds are commands, paths, variables, and application completions. The menu uses row-major order, at most ten rows including the border, and scrolls as the selection moves. The selected candidate is highlighted with reverse video. Items that are wider than the menu are clipped with `…`. When candidate descriptions are available, the menu is drawn as a single-column list with the description beside the candidate.

## Escape commands

A line beginning with `:` at the interactive prompt is an escape command. It is not passed to the shell or scripting language.

| Command | Effect |
| --- | --- |
| `:blood` | Print the embedded Blood brand art. |
| `:cfg.SECTION.KEY` | Show the current value and, for enumerated keys, all accepted values. Enumerated keys include `prompt.theme`, `prompt.directory_style`, boolean settings, and colours. |
| `:cfg.SECTION.KEY = VALUE` | Update a configuration key in the running session and immediately rewrite the corresponding line in `blood.ini`. Supports `prompt`, `colors`, `history`, `completion`, `shell`, `aliases`, and `env`. |
| `:ft SEARCH` | Recursively find files whose name contains `SEARCH` (case-insensitively) under the current directory and print their absolute paths. Entries named `.` and `..` are skipped. |
| `:ls [PATH]` | List a directory as a Unicode table with permissions, human-readable sizes, modification times, and creation times. Defaults to the current directory. Hidden entries are omitted, and directories are shown with a trailing `/`. |
| `:privacy` | Toggle hiding the user name, host name, and directory in the prompt. Prints `privacy: on` or `privacy: off`. |
| `:rla` | Reload `blood.ini`, re-seed aliases and environment variables, reset the interactive script environment, and run `.bloodrc` again. The prompt and colours update immediately. |
| `:version` or `:v` | Print the version and build hash. |
| `:where` | Print the paths of `blood.ini` and `.bloodrc`. |
| `:help` or `:` | Print the escape-command summary. |

Examples of escape commands follow.

```text
:cfg.prompt.theme
:cfg.prompt.theme = hacker
:cfg.colors.primary = red
:cfg.history.max_entries = 500
:cfg.aliases.ll = ls -l
:ft foo bar
```

Saving a key creates the `blood.ini` file and the containing section when needed. Existing comments and unrelated lines are preserved.

An unknown escape command prints `blood: unknown escape command: NAME`, prints the summary, and returns status 2. A `:ft` search that matches no files prints `blood: :ft: no files matching SEARCH` and returns status 1.

## Reverse incremental search

Ctrl-R enters reverse incremental search. The prompt becomes the following.

```text
(reverse-i-search) `QUERY': MATCH
```

Characters extend the query. Ctrl-R searches for the next older match. Backspace removes a query character. Enter accepts the matched line. Ctrl-C or Esc abandons the search and restores the line that was being edited when the search started. Search is case-insensitive and matches substrings of history entries. It searches from newest to oldest.

## Multiline input

The interactive shell collects input in three principal situations. A script line with unbalanced parentheses, brackets, braces, or quotes is collected until balance is restored. A line ending in a backslash is continued, and the backslash-newline sequence is removed by the command scanner. A script line whose final byte is `:` enters a block-collection mode in which lines are collected until a blank line is entered; because a trailing colon is not valid Blood syntax, the collected text is subsequently reported as a syntax error.

The continuation prompt is a dim `  … ` marker. A cancelled continuation abandons the whole pending command. A complete multiline command is recorded as one history entry.
