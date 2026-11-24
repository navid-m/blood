# Invocation

### Overview of invocation modes

The executable is invoked in one of the following forms.

```text
blood
blood -c COMMAND [ARG...]
blood SCRIPT.blood|SCRIPT.bx [ARG...]
blood -init
blood -login
blood -fmt PATH...
blood -doc [-md] [-o DIR] PATH...
blood -translate-sh [-o DIR] PATH...
blood -lsp
blood -ref [-stdout]
blood -version
blood -help
```

`blood` with no operands starts an interactive shell when standard input is a terminal. If standard input is not a terminal, Blood reads standard input to end of file and executes the result. The input is executed as a script when its first line has the shape of a scripting statement, and otherwise as a shell command.

`blood -c COMMAND` executes `COMMAND` and exits. If the first non-empty line of `COMMAND` has the shape of a scripting statement, the command is executed as a script. Otherwise it is executed as a shell command. Remaining positional arguments are exposed to the script as the list variable `args`.

`blood SCRIPT.blood ARG...` or `blood SCRIPT.bx ARG...` reads and executes the named script. Remaining arguments are exposed to the script as the list variable `args`. The script name is exposed as `__file__`. A missing script reports `blood: PATH: no such script` and exits with status 1.

`blood -translate-sh PATH...` translates bash scripts to Blood scripts. Each `PATH` may be a `.sh` or `.bash` file or a directory. Directories are walked recursively, and every bash script below them is translated. By default the translated file is written beside its source with the extension replaced by `.bx`; `-o DIR` (also `-out DIR`) writes the results below `DIR`, preserving the directory structure relative to the translated directory. The command prints one `SOURCE -> TARGET` line per translated file and exits with status 1 when an input cannot be read or written.

`blood -fmt PATH...` formats Blood scripts in place. Each `PATH` may be a file or a directory. Directories are walked recursively, and every `.bx` or `.blood` file below them is formatted. The formatter normalises indentation to four spaces, places opening braces on the same line as their header, removes Allman-style brace lines, aligns consecutive assignments, `var` declarations (including bulk `var` blocks), and type fields, and writes calls with parentheses where the call is unambiguous. Directories named `.git`, `node_modules`, and `.sxb` are skipped. Files that cannot be read or written are reported and produce status 1.

`blood -doc PATH...` generates HTML documentation for Blood scripts. Each `PATH` may be a `.bx` or `.blood` file or a directory. Directories are walked recursively, and only Blood files are processed. `-md` selects Markdown output instead of HTML. Output is written below `bx-docs-out` by default; `-o DIR` (also `-out DIR`) selects another root. The source path is preserved below the output root and the extension is replaced by `.md` or `.html`. Contiguous `##` comment lines immediately above a declaration become its description. The command prints one `SOURCE -> TARGET` line per generated file and exits with status 1 when an input cannot be read, parsed, or written.

`blood -lsp` runs the Blood language server over standard input and standard output. The server uses the Language Server Protocol with standard `Content-Length` framing. It keeps an in-memory copy of every open document and provides diagnostics, completion, hover, definitions, references, document and workspace symbols, rename, signature help, folding ranges, and formatting. The server runs until the client shuts it down or closes the streams.

`blood -init` writes a default configuration file at the configuration path when no file exists there. Parent directories are created. An existing file is left unchanged.

`blood -login` initializes a login-shell environment. On macOS the shell runs `/usr/libexec/path_helper` and merges the reported paths into `PATH`. On Linux the shell reads `/etc/environment` and merges its values, giving precedence to variables that are already set and merging `PATH` without duplicates. On Windows the inherited user environment is used. A login environment is also selected when the program name in `argv[0]` is prefixed by `-`, or when `[shell] login` is enabled in the configuration file.

`blood -version` and `blood -v` print the version line, which contains the program name, the release version, and the build hash.

`blood -help` and `blood -h` print usage information.

`blood -ref` writes the language reference embedded in the executable to `BLOOD_LANGUAGE_REFERENCE.adoc` in the current directory. With `-stdout`, the reference is written to standard output instead.

An unrecognised flag beginning with `-` produces `blood: unknown flag 'OPTION'`, prints usage information, and exits with status 2. Command-line options use the Plan 9 single-dash form. Parsing stops at the first positional argument, so script arguments beginning with `-` are passed through unchanged. Flags may be combined only as separate arguments; the parser does not support clustered short flags.

## Process model

The shell does not replace itself when executing a command. On POSIX systems, external commands are children created with `fork` and replaced with `execvp`. Pipelines create one child per stage. Background commands create a wrapper child. On Windows, external commands are created with the native process API; the observable model is otherwise equivalent.

Builtins run in the shell process unless they occur in a pipeline or a background command, in which case they run in the child.

The shell never invokes `/bin/sh` for ordinary command execution. A shell command is parsed and executed directly. `/bin/sh` is used only by external programs that choose to invoke it.

## Exit status

The following exit statuses are defined.

| Status | Meaning |
| --- | --- |
| 0 | Success. |
| 1 | General failure, including failed redirection, failed `cd`, a runtime script error, or a failed `source`. |
| 2 | Command syntax error, script syntax error, or invalid `exit` argument. |
| 126 | Reserved for a non-executable command when reported by the platform. |
| 127 | Command not found, or `exit` with a failed process launch. |
| 128+N | Command terminated by signal N. |
| 130 | Interrupted by SIGINT. |

A pipeline returns the status of its last stage. `&&` and `||` use the status of the previous segment. A background command returns 0 in the parent.

## Environment

The shell inherits the process environment. Shell variables and exported environment variables are related but distinct. A plain assignment such as `NAME=value` creates or updates a shell variable. `export NAME=value` creates or updates a shell variable and the process environment. A command prefix such as `NAME=value command` sets the variable in the child environment for that command; for builtins the assignment is applied temporarily and then restored. `unset NAME` removes the shell variable and the environment variable.

Variable lookup first checks shell variables and then the process environment. The shell seeds `PATH`, `HOME`, and `SHELL` from the inherited environment at startup. On Windows, `HOME` falls back to `USERPROFILE`.

## Signals

The interactive shell installs a SIGINT handler that sets an interrupt flag. SIGQUIT, SIGTERM, and SIGTSTP are ignored by the shell process. External children restore the default dispositions for SIGINT, SIGQUIT, SIGTERM, SIGTSTP, SIGTTIN, and SIGTTOU before `exec`.

While the line editor is active the terminal is in raw mode and the ISIG flag is disabled, so Ctrl-C and Ctrl-Z arrive as ordinary bytes. While a command is executing the original terminal settings are restored, so Ctrl-C is delivered to the foreground child. When the child terminates, the shell resumes editing with a new prompt.

A script can be interrupted at a loop boundary or before or after a shell command. An external command interrupted by Ctrl-C normally produces status 130.
