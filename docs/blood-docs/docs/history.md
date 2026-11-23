# History

History is stored one entry per line. The file format is not intended to be edited by hand, although ordinary single-line commands appear verbatim. Within an entry, a backslash is encoded as `\\`, a line feed as `\n`, and a carriage return as `\r`.

History is loaded at interactive startup, appended as commands are accepted, and rewritten in full when the shell exits. Entries beyond `max_entries` are removed from the oldest end. Consecutive duplicates are suppressed when `ignore_duplicates` is true.

The `history` builtin prints entries with one-based indices.

```text
 1  echo one
 2  echo two
```

`history -c` clears both memory and file. `history -w` rewrites the file. `history N` prints the last N entries. A non-numeric argument is treated as a request for all entries. When `enabled` is false, the history list is neither loaded nor recorded, and the `history` builtin reports an empty list.

When `[completion] history_suggestions` is enabled, the editor offers the newest history entry that extends the current line and contains no line break. The suggestion is case-sensitive according to `[completion] case_sensitive` and is accepted with the Right or End key. The suggestion is never recorded as a separate history entry.
