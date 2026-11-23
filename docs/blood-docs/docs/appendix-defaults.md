# Appendix: default settings

The following table lists the built-in defaults, which apply when a key is absent from `blood.ini`. The file written by `blood -init` is shown in [Configuration](configuration.md).

| Section | Key | Default |
| --- | --- | --- |
| prompt | theme | `default` |
| prompt | show_username | `true` |
| prompt | show_hostname | `true` |
| prompt | show_directory | `true` |
| prompt | show_git | `true` |
| prompt | show_time | `false` |
| prompt | show_exit_status | `true` |
| prompt | show_duration | `true` |
| prompt | show_jobs | `true` |
| prompt | show_mode | `false` |
| prompt | two_line | `true` |
| prompt | directory_style | `full` |
| prompt | prompt_symbol | `❯` |
| prompt | prompt_char | `❯` |
| prompt | separator | `─` |
| colors | primary | 256-colour 124 |
| colors | secondary | 256-colour 88 |
| colors | success | `green` |
| colors | error | `red` |
| colors | directory | 256-colour 88 |
| colors | git_clean | `green` |
| colors | git_dirty | 256-colour 88 |
| colors | muted | `bright_black` |
| colors | warning | `yellow` |
| colors | accent | `bright_cyan` |
| history | enabled | `true` |
| history | file | `~/.local/share/blood/history` |
| history | max_entries | `10000` |
| history | ignore_duplicates | `true` |
| completion | enabled | `true` |
| completion | case_sensitive | `false` |
| completion | history_suggestions | `false` |
| completion | external | `false` |
| shell | default_editor | `nano` |
| shell | login_message | `true` |
| shell | login | `false` |
| shell | startup_file | empty |

The keys `git_status`, `symbol`, `history_file`, `history_max`, `autosuggestions`, and `editor` are accepted aliases. Their defaults follow the canonical keys listed above. The `dir` alias resolves to `ls` by default.
