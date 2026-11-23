# Terminology

The words *shall*, *must*, *should*, and *may* are used in their conventional normative sense. A *shell command* is a command entered in the interactive shell or executed by the command interpreter. A *script* is a sequence of Blood language statements. A *line* is a sequence of bytes terminated by a line feed or by end of input.

The shell has two execution domains. The command domain is described in [Command language](command-language.md), and the scripting domain is described in [Scripting language](scripting-language.md). Both domains share the same process, working directory, environment, and file descriptors. A script may execute shell commands, and a shell command may run a script.
