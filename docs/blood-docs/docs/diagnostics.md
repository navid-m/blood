# Diagnostics

## Configuration diagnostics

Configuration diagnostics are written to standard error and do not stop the shell.

```text
blood: config: /path/to/blood.ini
  line N: expected key = value
blood: config: invalid boolean 'VALUE'
blood: config: invalid integer for KEY
```

## Shell diagnostics

Shell diagnostics are written to standard error.

```text
blood: NAME: command not found
blood: PATH: cannot open for reading
blood: PATH: cannot open for writing
blood: cd: PATH: no such directory
blood: source: PATH: no such file
blood: alias: NAME: not found
blood: exit: numeric argument required
blood: ls: PATH: no such directory
blood: fork failed
```

Syntax errors in the command language produce status 2. Unknown commands produce status 127. Failed redirections and failed builtins produce status 1.

## Script diagnostics

Script syntax and runtime diagnostics have the following form.

```text
blood: LABEL:LINE:COL: MESSAGE
```

For interactive submissions the label is empty, yielding the following form.

```text
blood: LINE:COL: MESSAGE
```

## Interactive messages

Ctrl-C while editing prints `^C` and returns to a new prompt. Ctrl-D on an empty line ends the session. A cancelled completion menu, a cancelled search, or a cancelled continuation does not execute a command. An unknown escape command prints the escape-command summary and returns status 2.
