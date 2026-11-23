# Performance characteristics

The implementation is designed to avoid unnecessary process creation and repeated work.

| Mechanism | Effect |
| --- | --- |
| Builtins outside pipelines | Execute in the shell process without a child. |
| External commands | Fork and exec directly; `/bin/sh` is not used as an intermediary. |
| Pipelines | Fork only the stages that are required. |
| Command substitution | Uses the Blood parser in a child process. |
| Prompt Git information | Cached for 1500 milliseconds for the current directory. |
| Completion executable index | Built lazily on first command completion and cached for the process lifetime. |
| Prompt rendering | Prompt text is computed once per command, not once per keystroke. |
| Line editor | Byte-oriented; redraws the prompt line and the open menu. |
| History | Appends one entry per command and rewrites the file on exit. |
| Parallel constructs | Bounded by the worker limit, which defaults to the number of logical processors. |

The implementation does not use an arena for shell command parsing. Temporary command structures and script values are allocated from the C heap and are not generally reclaimed during the lifetime of the process. This is a known limitation of version 0.1.5.
