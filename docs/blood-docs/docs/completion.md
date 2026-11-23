# Completion

Completion is computed for the token containing the cursor. The token starts after whitespace or one of `|`, `&`, `;`, `<`, `>`, `(`, or `)`.

Completion is selected as follows. If no earlier word occurs in the current command segment, completion is for command names; a word that contains `/` or begins with `.` or `~` is completed as a path instead. If the command is `cd`, `pushd`, or `popd`, completion is for directories. If the command is `export`, `unset`, or `set`, completion is for shell variable names. Otherwise completion is for filesystem entries.

Command completion considers builtin command names, alias names, and executables found in the directories of `PATH`. The `PATH` scan is performed lazily on the first command completion and cached for the life of the process. Only entries that are not directories and that pass the executable access check are included.

When `[completion] external` is enabled, argument completion for a non-builtin command is delegated to the Bash programmable-completion engine. Blood loads the command's `complete` definition from the standard Bash completion directories and from `~/.bash_completion`, evaluates it in a child Bash process with a 500 millisecond limit, and uses the resulting `COMPREPLY` candidates. A candidate may include a description when the Bash definition produces a tab-separated value and description. Path-like words and commands with no matching Bash definition continue to use Blood's filesystem completion.

Filesystem completion splits the word at its final `/`. The directory portion is expanded for `~` and listed. Entries whose names begin with `.` are offered only when the typed basename begins with `.`. Directories receive a trailing `/`. Candidates preserve the typed directory prefix. Typing `./some/path/` and pressing Tab therefore lists the contents of that directory in the completion menu.

Completion inside a quoted word is not attempted when the character before the cursor is `"` or `'`.

Builtin, path, and variable candidates are sorted lexicographically. Externally completed candidates are kept in the order produced by Bash. Duplicate candidates are removed. The menu title identifies the candidate kind: commands, paths, variables, or application completions.

When `[completion] history_suggestions` is enabled, an inline suggestion from history is drawn after the cursor and accepted with the Right or End key. This behaviour is described further in [History](history.md).
