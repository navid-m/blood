# bloodshell

### Purpose

This set of documents specifies the Blood shell as implemented by version 0.1.5. It describes the observable behaviour of the interactive shell, its command language, its configuration system, and the Blood scripting language. It is a specification of the implementation rather than a statement of future intent. Where the implementation is deliberately permissive or incomplete, that is stated explicitly.

### Implementation Summary

The implementation is written in the Spectre Programming Language and is built with the Spectre toolchain version recorded in the project's sx.mod. Blood builds for Linux, macOS, and Windows. The POSIX builds target a POSIX-style C library and use dynamic linking. The Windows build uses the native process and console APIs. Unless this document states otherwise, the specified behaviour is the same on every supported platform.

### Documentation structure

The documentation/specification follows the structure of the implementation. The command domain and the scripting domain are specified separately, and the interactive terminal, prompt, history, and completion subsystems are specified before the languages that use them.
