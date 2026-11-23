# Command language

## Input decomposition

A shell input is split into segments at top-level occurrences of `;` and line feed, which are unconditional separators; `&&`, which executes the next segment only when the previous status is zero; `||`, which executes the next segment only when the previous status is non-zero; and `&`, which executes the segment in the background.

Splitting respects single quotes, double quotes, backslash escapes, backticks, and `$(...)`. A `#` that begins a word starts a comment that extends to the end of the line.

A segment is split into a pipeline at top-level `|` characters. A pipeline is a sequence of commands. Consecutive pipelines form a command list.

## Words and quoting

A word is a maximal sequence of bytes not containing unquoted whitespace or one of the metacharacters `|`, `&`, `;`, `<`, or `>`.

Within an unquoted word, a backslash quotes the next byte, and a backslash followed by a line feed is a line continuation in which both bytes are removed. Single quotes preserve every byte until the next single quote. Double quotes preserve every byte until the next double quote, except that a backslash escapes `"`, `\`, `$`, and backtick, and line continuations are removed. `$` and backtick introduce expansions, and a leading `~` introduces tilde expansion.

An empty quoted word is a valid empty argument.

## Expansions

### Variables

| Form | Meaning |
| --- | --- |
| `$NAME` | Value of shell variable or environment variable `NAME`. |
| `${NAME}` | Same, with explicit delimiters. |
| `$?` | Status of the previous command. |
| `$$` | Process ID of the shell. |
| `$!` | Process ID of the most recent background command. |
| `$#` | Number of script arguments. |
| `$0` | Program name, normally `blood`. |

An undefined variable expands to the empty string. `$@`, `$*`, and array or positional expansion are not implemented and are left literal or partially expanded as ordinary text.

Variable expansion is performed before field splitting. A variable whose value contains spaces expands to a single word; the shell does not perform word splitting on expansion results.

### Tilde expansion

A word that begins with `~` has the `~` replaced by the value of `HOME`. `~user` is not resolved through the user database; any `~` followed by characters other than `/` or the end of the word is left unchanged.

### Command substitution

Command substitution has the following two forms.

```text
$(COMMANDS)
`COMMANDS`
```

The commands are executed in a child process with standard output captured. Trailing line feeds and carriage returns are removed from the result. The result is inserted into the current word as a single field; it is not split on whitespace. The status of the substitution becomes the shell's previous status. The child runs Blood's own command interpreter, not `/bin/sh`.

### Globbing

An unquoted word containing `*`, `?`, or `[` is treated as a pathname pattern. The pattern is expanded against the filesystem. If no match exists, the word is left unchanged.

| Pattern element | Meaning |
| --- | --- |
| `*` | Matches any sequence of bytes. |
| `?` | Matches one byte. |
| `[abc]` | Matches one byte in the set. |
| `[a-z]` | Matches one byte in the range. |
| `[!abc]`, `[^abc]` | Matches one byte not in the set. |

A leading dot in a filename is matched only when the pattern component also begins with a dot. Glob results are sorted lexicographically. Brace expansion, recursive `**`, and extglob are not implemented. Assignment words are not globbed.

## Assignments

An assignment word has the form `NAME=VALUE`, where `NAME` begins with an ASCII letter or underscore and continues with letters, digits, or underscores.

If assignments occur before the command name and no command name is present, the assignments update shell variables. If a command follows, the assignments are placed in the child environment for that command. For builtins, the assignments are temporarily applied and then restored.

`export NAME=VALUE` updates both the shell variable and the environment. `export NAME` exports the current value.

## Aliases

`alias NAME=VALUE` defines an alias. `alias NAME` prints one alias. `alias` with no arguments prints all aliases. `unalias NAME...` removes aliases. `unalias -a` removes all aliases.

Alias names may contain punctuation such as dots. A definition value may contain spaces without quotes: words after `NAME=` are appended until another `NAME=` token starts a new definition.

```text
alias cd..=cd ..
```

Blood defines the convenience alias `dir` for `ls` before configuration aliases are installed. It may be removed or replaced like any other alias.

Aliases are expanded by re-scanning the replacement text as a shell line with the original arguments appended and quoted. Expansion is recursive up to a depth of eight. Alias expansion is performed for single-command input. An alias used as a stage of a multi-command pipeline is currently not expanded and is looked up as an external command.

Aliases defined in configuration are loaded before startup scripts.

## Redirections

| Form | Meaning |
| --- | --- |
| `< FILE` | Redirect standard input from `FILE`. |
| `> FILE` | Truncate `FILE` and redirect standard output. |
| `>> FILE` | Append standard output to `FILE`. |
| `2> FILE` | Truncate `FILE` and redirect standard error. |
| `2>> FILE` | Append standard error to `FILE`. |
| `&> FILE` | Truncate `FILE` and redirect both standard output and error. |
| `&>> FILE` | Append both standard output and error to `FILE`. |
| `2>&N` | Duplicate file descriptor N onto standard error. |

A redirection target is a word and undergoes normal expansion. Redirections apply left to right. A failed open reports an error and the command is not run or runs with status 1. Redirections on a builtin are applied in the shell process and the original descriptors are restored afterwards.

Heredocs, here-strings, process substitution, and arbitrary `N>` forms other than the listed forms are not implemented.

## Pipelines and background execution

`A | B` creates a pipe between A and B. Commands in a pipeline run concurrently. Builtins in a pipeline run in their stage child. The pipeline status is the status of its last stage.

`COMMAND &` runs the entire pipeline in a wrapper child. The wrapper's process ID is recorded as a background job. `$!` is set to that ID. `jobs` lists running jobs and `wait` waits for all recorded jobs.

An unquoted `time` at the beginning of a pipeline is a shell keyword. It runs the pipeline and then writes timings to standard error in the same shape as `bash`.

```text

real    0m1.201s
user    0m1.067s
sys     0m0.133s
```

The pipeline's output is not delayed or captured. Redirections on the pipeline do not redirect the timing report. To run the external `time` utility, quote the name or use its path, for example `'time'` or `/usr/bin/time`.

## Command resolution

A command name containing `/` is passed directly to the platform executable loader.

A command name without `/` is resolved using `PATH`. Builtins are checked before the executable search.

If execution fails, the child writes the following message and exits with status 127.

```text
blood: NAME: command not found
```

A command that consists of a single word naming an existing directory, with no arguments, assignments, or redirections, changes the working directory. The word must contain `/` or begin with `/`, `.`, or `~`. This is equivalent to `cd` for that directory.

## Builtin commands

| Command | Behaviour |
| --- | --- |
| `cd [DIR]` | Change directory. With no argument, use `HOME`. `cd -` returns to `OLDPWD` and prints the new directory. Updates `PWD` and `OLDPWD`. |
| `pwd` | Print the current working directory. |
| `exit [CODE]` | Terminate the shell with `CODE`, or with the previous status when `CODE` is omitted. A non-numeric code reports `blood: exit: numeric argument required` and uses 2. |
| `quit` | Alias for `exit`. |
| `history [-c\|-w\|N]` | Show or manage history as described in [History](history.md). |
| `clear` | Write the ANSI clear-screen sequence. |
| `cls` | Alias for `clear`. |
| `export [NAME=VALUE...] [NAME...]` | Export variables. With no arguments, print shell variables. |
| `unset NAME...` | Remove shell and environment variables. |
| `alias [NAME=VALUE...] [NAME...]` | Define or print aliases. |
| `unalias NAME...` | Remove aliases. `-a` removes all. |
| `help [TOPIC]` | Print help. Topics are `config`, `script`, and `language`. |
| `source FILE` | Execute a Blood script in the current shell state. |
| `.` | Alias for `source`. |
| `echo [-n] ARG...` | Print arguments separated by spaces. `-n` suppresses the trailing newline. |
| `say ARG...` | Print arguments separated by spaces and a newline. |
| `set [NAME=VALUE...]` | With no arguments, print shell variables. Otherwise assign values. |
| `type NAME` | Resolve a shell keyword, builtin, alias, or `PATH` entry. |
| `which NAME` | Same as `type`. |
| `jobs` | Reap finished jobs and print running background jobs. |
| `wait` | Wait for all recorded background jobs and clear the job list. |
| `true` | Return 0. |
| `false` | Return 1. |
| `:` | Return 0. |
| `ls [PATH]` | Windows only. List a directory in short form, or in long form with `-l`. Unix builds use the external `ls` command. |

`type` checks shell keywords first, then builtins, then aliases, then `PATH`. It does not verify that a `PATH` candidate is executable. Only the first argument is used. `source` and `cd` use only their first argument.

## Shell grammar

The following grammar describes the command language accepted by the implementation. `WORD` is a word after quoting and expansion, and `SEP` is a top-level separator.

```text
shell-input  = { shell-list } ;
shell-list   = segment { connector segment } ;
connector    = ";" | NEWLINE | "&&" | "||" | "&" ;
segment      = pipeline ;
pipeline     = [ "time" ] command { "|" command } ;
command      = { assignment } { WORD | redirection } ;
assignment   = NAME "=" WORD ;
redirection  = "<" WORD
             | ">" WORD
             | ">>" WORD
             | "2>" WORD
             | "2>>" WORD
             | "&>" WORD
             | "&>>" WORD
             | "2>&" DIGITS ;
NAME         = ( letter | "_" ) { letter | digit | "_" } ;
DIGITS       = digit { digit } ;
```

The scanner removes comments beginning with `#` at a word boundary, performs quote removal, variable expansion, tilde expansion, command substitution, and globbing, and then yields `WORD` tokens plus operator tokens. The parser groups these tokens into the structures above.
