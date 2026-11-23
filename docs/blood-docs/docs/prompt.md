# Prompt

## Default two-line layout

The default prompt has the following form.

```text
┌─ USER@HOST DIRECTORY on BRANCH *
└─ ❯
```

The first line begins with a muted `┌─ `. The second line begins with a muted `└─ ` followed by the prompt character in the primary colour. When `[prompt] two_line` is false, the components are printed on one line followed by the prompt character.

Components are emitted in the following order: the user name, when enabled; `@` and the host name, when both user name and host name are enabled, or the host name alone when only the host name is enabled; the directory, when enabled; the Git branch, when enabled and a repository is found; the time, when enabled; the background-job count, when enabled and non-zero; the failure status, when enabled and the previous status is non-zero; and the previous command duration, when enabled and at least 500 milliseconds.

Failure status is rendered as a red `✘ N`. Duration is rendered as a muted `took Nms`, `took N.Ns`, or `took NmNs`. Background jobs are rendered as a yellow `⚙ N`. A dirty Git branch appends a ` *` marker in the dirty colour.

Privacy mode is not a configuration key. It is toggled by the `:privacy` escape command and temporarily suppresses the user name, host name, and directory for prompt rendering until it is toggled again.

## Plain theme

`theme = plain` selects a single-line prompt of the following form.

```text
USER@HOST DIRECTORY (BRANCH) >
```

The plain theme always uses `>` and does not print a box. It renders the user name and, when the user name is enabled, the host name, followed by the directory and the Git branch. It does not render time, status, duration, job count, or the configured prompt character.

## Prompt presets

A preset is selected with `[prompt] theme = NAME`. Every preset honours the `show_*` toggles and the `[colors]` settings. Presets define their own glyphs and path style.

| Theme | Layout |
| --- | --- |
| `lambda` | `λ DIRECTORY BRANCH ›`, where the directory is the final path component. |
| `powerline` | Coloured powerline segments for user, host, directory, branch, time, and indicators, followed by a second `❯` line. |
| `right` (also `right1`, `right_aligned`) | `DIRECTORY>` followed by a right-aligned `USER@HOST`. The cursor stays after the left part. |
| `right_time` (also `right2`, `right_aligned_time`) | `DIRECTORY ❯` followed by a right-aligned branch, clean or dirty mark, and `HH:MM` time. |
| `hacker` | `USER :: HOST :: DIRECTORY :: BRANCH` followed by a `>>>` line. The host name is lowercase and the directory is the final path component. |
| `dots` | `● USER  ● DIRECTORY  ● BRANCH` followed by a `❯` line. |
| `compact` (also `compact_path`) | A compacted home path such as `~/D/p/blood`, the branch, and `❯`. |
| `iconified` (also `icons`) | A three-line `╭─ ☼ USER on HOST`, `├─ ⌂ DIRECTORY`, `╰─ ⎇ BRANCH ➜` block. |
| `timestamped` (also `timestamp`) | `[HH:MM] USER@HOST DIRECTORY (BRANCH)` followed by a `$` line. The time is shown only when `show_time` is enabled. |
| `doubleline` (also `double`) | A three-line `╔═ USER@HOST`, `╠═ DIRECTORY`, `╚═ BRANCH ═▶` block. |

The `powerline` preset converts the configured foreground colours into segment backgrounds and uses Powerline separator glyphs. It requires a font with Powerline glyphs.

The two right-aligned presets draw their right-hand text at the terminal edge and then return the cursor to the end of the left-hand prompt. When the terminal is too narrow for both parts, the right-hand text is omitted and the input uses the full remaining line.

## Directory formatting

The home directory prefix is replaced by `~` when it matches a complete path component. `full` preserves the remaining path. `short` keeps the last two components. `basename` keeps only the last component. When the home directory itself is displayed, it is shown as `~`.

## Git information

Git discovery walks from the current directory toward the filesystem root. At each level it checks for a `.git` directory. If `.git` is a file, a `gitdir: PATH` line is followed and relative paths are resolved against the directory containing the `.git` file.

The branch is read from `HEAD`. A symbolic reference `ref: refs/heads/NAME` yields `NAME`. Another symbolic reference yields the text after `ref: `. A detached HEAD yields the first seven characters of the commit hash.

Dirty state is obtained by running the following command.

```text
git -C ROOT status --porcelain --untracked-files=no
```

A non-empty result marks the branch dirty. Untracked files are therefore not considered. If `git` is unavailable or fails, the repository is treated as clean. Git information is cached for 1500 milliseconds for the current directory, so the prompt does not spawn `git` on every keystroke.
