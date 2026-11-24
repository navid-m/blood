# Scripting language

## Source files

A Blood script is a sequence of bytes. The conventional extensions are `.blood` and `.bx`. Extensions are not enforced. A leading shebang line is treated as a comment because it begins with `#`.

Scripts are executed by `blood SCRIPT.blood` or `blood SCRIPT.bx`, by the `source` shell builtin, by the script `source` function, by `startup_file`, and by the interactive shell when a submitted line has the shape of a script statement.

Interactive submissions retain their global variables and function definitions for the lifetime of the shell. `source` and script files executed from the shell share the same global environment.

## Lexical structure

### Encoding and identifiers

Source is a byte sequence. Identifiers are ASCII.

```text
IDENT = ( letter | "_" ) { letter | digit | "_" } ;
letter = "A".."Z" | "a".."z" ;
digit  = "0".."9" ;
```

Identifiers are case sensitive. Keyword matching is case sensitive.

### Whitespace and comments

Spaces and tabs separate tokens. A `#` outside a string begins a comment that extends to the next line feed. Blank lines and comment-only lines do not affect block structure.

### Numeric literals

```text
INT   = digit { digit } ;
FLOAT = digit { digit } ( "." digit { digit } [ exponent ] | exponent ) ;
exponent = ( "e" | "E" ) [ "+" | "-" ] digit { digit } ;
```

Integer literals are decimal signed 64-bit values. Floating-point literals use the platform double representation. Hexadecimal, binary, octal, digit separators, and suffixes are not implemented. A leading decimal point is not part of a numeric literal; `.5` is not a float.

### String literals

Strings are delimited by either single or double quotation marks. Both forms behave identically: escape processing and interpolation apply to both.

| Escape | Meaning |
| --- | --- |
| `\n` | Line feed. |
| `\t` | Horizontal tab. |
| `\r` | Carriage return. |
| `\0` | NUL byte. |
| `\xHH` | Byte with hexadecimal value HH. |
| `\"` | Double quote. |
| `\'` | Single quote. |
| `\\` | Backslash. |

A backslash before any other byte yields that byte. A literal line feed is accepted inside a string.

String interpolation occurs for both quote forms. `{` begins an interpolation field. The field ends at the matching `}`. Braces and quotes inside the field are balanced by the interpolation scanner. `{{` produces a literal `{` and `}}` produces a literal `}`. The text between the braces is parsed as a Blood expression and evaluated in the current environment; its value is converted to a string and inserted.

Interpolation errors use the location information of the inner expression and may not reflect the outer source line.

A raw string is written `r"..."` or `r'...'`. Raw strings perform no escape processing and no `{...}` interpolation, so backslashes and braces are taken literally. This is useful for regular expressions and other patterns that are dense in backslashes.

A multiline string is delimited by three matching quotes, `"""..."""` or `'''...'''`. Escape processing and interpolation apply as for other quoted strings, and line feeds inside the literal are preserved. Triple quotes may contain unescaped single and double quotes. When the opening delimiter is followed by a line feed, that first line feed is ignored. When the closing delimiter is also on a line by itself, the line feed before it and the indentation shared by the content lines are removed.

```blood
sh("""
cat first.txt
cat second.txt
""")
```

### Command substitution literals

Outside strings, `` `COMMANDS` `` and `$(COMMANDS)` denote command substitution. They are lexed as a single expression token. The commands are executed by the shell in a child process, trailing newlines are removed, and the result is a string value. The completion status becomes the value of the script variable `status`.

### Blocks

Blocks are introduced by braces. The opening brace follows the header of a compound statement and the matching closing brace ends its body.

```text
if CONDITION {
    BLOCK
}
```

Newlines and indentation are ignored inside unmatched `(` and `[` delimiters and inside dictionary literals. This permits multiline lists, dictionaries, and call arguments.

A method or property chain cannot continue with `.` at the start of a new line. Keep the dot on the previous line or wrap the chain in parentheses.

```blood
var files = std.glob("./*").filter(
    |f| f.basename().like("*.txt")
)
```

A semicolon `;` may separate simple statements on one line. A single-line suite may also follow the opening brace. A trailing colon does not introduce a block; Python-like suites are rejected with a syntax error.

### Keywords

The following identifiers are treated as keywords at the start of a statement or in expression position.

```text
if elif else while for in para fn return break continue pass
try catch except raise throw say defer export true false
null none not and or var const pub use reduce
```

`none` is accepted as an alias for `null`.

## Values

The runtime value types are as follows.

| Type | Representation and behaviour |
| --- | --- |
| null | The absence of a value. Written `null` or `none`. |
| bool | `true` or `false`. Integers and floats are truthy when non-zero. |
| int | Signed 64-bit integer. |
| float | IEEE 754 double. |
| string | Byte string. Strings are immutable; operations produce new strings. |
| list | Ordered, mutable sequence of values. |
| dict | Mutable string-keyed map. Iteration order is unspecified. |
| function | User-defined or native callable. |
| namespace | Named bindings exposed by a module or by `std`. |

Lists and dictionaries are reference values. Assigning a list or dictionary to another variable aliases the same object; there is no implicit copy.

## Expressions

### Precedence

Precedence, from lowest to highest, is as follows.

| Level | Operators |
| --- | --- |
| 1 | `??` |
| 2 | `or` |
| 3 | `and` |
| 4 | `not` (prefix) |
| 5 | `==` `!=` `<` `>` `<=` `>=` `in` `not in` |
| 6 | `..` `..=` |
| 7 | `+` `-` |
| 8 | `*` `/` `%` |
| 9 | unary `-` and unary `+` |
| 10 | call `(...)`, index `[...]`, attribute `.NAME` |

Only one comparison operator is accepted at a given level; chained comparisons such as `a < b < c` are not supported. Parentheses override precedence.

`??` returns its left operand unless that operand is null, in which case it evaluates and returns the right operand. It does not treat false or empty values as missing.

`a..b` produces the list of integers from `a` up to but not including `b`. `a..=b` includes `b`. A range whose start is greater than its end is empty.

### Primary expressions

Primary expressions are integer, float, and string literals; `true`, `false`, `null`, and `none`; variable references; parenthesised expressions; list literals; dictionary literals; conditional expressions; and command substitutions.

List literals have the form `[e1, e2, ...]`. A leading `...` spreads a list into the literal, as in `[first, ...rest]`. Dictionary literals have the form `key1: value1, key2: value2, ...` inside braces.

A conditional expression has the form `if CONDITION { ... } elif CONDITION { ... } else { ... }`.

### Calls, indexing, and attributes

Calls have the form `callee(arg1, arg2, ...)`. A trailing comma is permitted. Arguments may be named as `name: value`; named arguments bind to the parameter with that name. User-defined functions, lambdas, and native functions all accept named arguments. Missing arguments use the parameter default when one is declared, and are otherwise bound as null. Extra arguments are ignored by user-defined functions.

Parentheses may be omitted when the call is unambiguous, as in `sh "echo hi"`, `say "hello"`, or `x = len args`. A command-shaped statement whose first word names a function is called as a function with the remaining text as its arguments; if no such function exists, the line is run as a shell command. Parentheses are still required for method calls, zero-argument calls, and calls whose arguments contain top-level operators.

Indexing has the form `container[index]`. Lists accept integers, including negative indices from the end. Dictionaries accept string keys; non-string keys are converted to strings. Strings accept a byte offset, including negative offsets, and return the UTF-8 sequence beginning at that offset.

Attribute access has the form `container.name`. On a dictionary it reads the value for key `name`. On a namespace it reads the exported binding named `name`. On a struct it reads a field or method. A missing attribute raises a catchable `unknown member` error, including when the access occurs inside string interpolation. Dictionary indexing with a missing key still yields null. Attribute assignment `dict.name = value` is supported for dictionaries and for mutable exported module variables.

### Operators

| Operator | Semantics |
| --- | --- |
| `+` | Numeric addition. If either operand is a string, concatenate their string representations. If both operands are lists, produce a new concatenated list. |
| `-` | Numeric subtraction. |
| `*` | Numeric multiplication. |
| `/` | Division. Integer division truncates when both operands are non-float. Division by zero raises a catchable error. |
| `%` | Integer remainder. Division by zero raises a catchable error. |
| `==`, `!=` | Equality. Nulls are equal. Numeric values compare across int, float, and bool. Strings compare by content. Lists compare deeply. Dictionaries and functions compare by identity. |
| `<`, `>`, `<=`, `>=` | If either operand is a string, compare string representations lexicographically. Otherwise compare numerically. |
| `and` | Short-circuit conjunction. Returns the left operand when it is falsy; otherwise returns the right operand. |
| `or` | Short-circuit disjunction. Returns the left operand when it is truthy; otherwise returns the right operand. |
| `not` | Logical negation, producing a bool. |
| `in` | Membership. Lists use equality. Dictionaries test keys. Strings test substrings. |
| `not in` | Negated membership. |

Unary `-` negates numerically. Unary `+` is accepted and has no effect.

### Truthiness

`null` and `false` are false. Numeric zero is false. The empty string, empty list, and empty dictionary are false. All other values are true.

### Interpolation

String interpolation is equivalent to evaluating each `{...}` field and concatenating its string representation. Each field is a complete expression, so calls, indexing, arithmetic, ranges, and null coalescing all work directly.

```blood
say "args: {args.len()}"
say "score: {scores[key] ?? 0}"
say "inclusive: {1..=5}"
```

The current lexical environment is used, including function parameters and local variables.

## Statements

### Assignment and declaration

```text
var NAME = expression
var (
    NAME = expression
    OTHER = expression
)
const NAME = expression
NAME = expression
PLACE += expression
PLACE -= expression
PLACE *= expression
PLACE /= expression
PLACE %= expression
```

Variables must be introduced with `var` or `const` before they can be assigned. A `var` declaration creates a mutable binding in the current scope and reports an error if the name is already declared there. A `const` declaration creates a read-only binding; assigning to it, including with a compound assignment, reports an error. The value stored by a constant may still be a mutable list or dictionary. The bulk form declares several variables at once. Declarations in a bulk block are separated by newlines or commas. Declarations made inside a `for` or `while` body are local to that body and are reset on each iteration.

Plain assignments do not create bindings. If the target variable does not exist in the current environment chain, evaluation reports `undefined variable`. If it exists in an enclosing scope, that binding is updated.

`PLACE` may be a variable, an index expression, or a dictionary attribute for plain and compound assignments. Assignments are statements, not expressions. Chained assignment is not supported.

Prefixing a variable assignment with `export` also writes the value to the process environment.

```blood
export PATH = (user_paths + [std.env("PATH")]).join(":")
```

The variable remains available as an ordinary script binding.

### Modules

```blood
use "some_file.bx"

say(some_file.PI)
some_file.some_function()
```

A `use` statement loads another Blood source file as a module. The module name is the file name without its final extension. A relative path is resolved against the directory of the file containing the `use` statement. Loaded modules are cached by absolute path, so repeated imports share the same module state.

Only top-level declarations marked `pub` are visible through the module namespace. Private variables and functions remain accessible inside the module but are not exported.

```blood
# some_file.bx
pub const PI = 3.14
pub var some_changeable_thing = 200

fn private_function() {
    say "hehe"
}

pub fn some_function() {
    private_function()
}
```

`pub` may prefix `var`, `const`, or `fn`. Exported mutable variables may be assigned through the namespace, and the change is visible to the module's functions. Exported constants and functions are read-only. A module is executed once, in its own top-level environment, when it is first imported.

### `say`

```text
say [expression {, expression}]
```

Evaluates each expression, converts each to a string, joins them with a single space, and writes the result followed by a newline to standard output. With no expressions, writes a newline. `say` is a statement, not a function.

### Conditionals

```text
if CONDITION {
    BLOCK
} elif CONDITION {
    BLOCK
} else {
    BLOCK
}
```

The first true branch is executed. `elif` is the only spelling for an additional condition; `else if` is not part of the language. A single-line suite after the opening brace is accepted.

A conditional is also an expression. In expression position it yields the value of the selected branch, defined as the value of that suite's final expression statement. A branch ending in any other statement, and a conditional where no branch matches, yields `null`. This allows conditionals to be used as arguments, initializers, and return values.

```blood
var label = if count > 1 { "items" } else { "item" }
exit(if failed.len() > 0 { 1 } else { 0 })
```

### Loops

```text
while CONDITION {
    BLOCK
}

for NAME in EXPRESSION {
    BLOCK
}

for KEY, VALUE in EXPRESSION {
    BLOCK
}
```

`while` reevaluates its condition before each iteration. The single-variable form of `for` iterates over the elements of a list, the UTF-8 code points of a string, or the keys of a dictionary. The two-variable form iterates a dictionary, binding the key to `KEY` and the corresponding value to `VALUE`; dictionary iteration order is unspecified. Use `.sorted()` or `.sorted_keys()` when deterministic key order is needed. `break` and `continue` affect the innermost loop.

`for` binds its loop variables in the current environment, and they remain visible after the loop. There is no C-style `for`, no `do`, and no `loop` statement.

### Parallel loops

```text
para for NAME in EXPRESSION {
    BLOCK
}
```

On POSIX platforms, `para for` evaluates `EXPRESSION` once and runs the body for each element in a separate child process. Iterations run concurrently, subject to the worker limit. Each child starts from a copy of the parent's variables, with `NAME` bound to its element and `status` reset to `0`; assignments made inside the body are visible only within that child and do not propagate back to the parent. Output from iterations may interleave.

After every iteration has finished, the loop waits for all children and assigns the first non-zero iteration status to `status`. An iteration's status is its final `status` value, or the argument passed to `exit`. An uncaught error in an iteration reports the error and makes that iteration fail without aborting the remaining iterations. `para for` does not support `break` or `continue`. On Windows, the statement runs the body serially in the parent process.

The number of concurrent workers defaults to the number of logical processors. `std.set_workers(n)` changes the limit for subsequent parallel constructs.

### Parallel map

```blood
var results = values.pmap(|x| f(x))
```

`pmap` is the parallel counterpart of `map`. It starts a child process for each element, subject to the worker limit, and returns a new list with the callback results in the original input order. Assignments and other side effects inside the callback stay in the child process; only the returned value is sent back to the parent. Values are copied between processes, so results must be data values: null, booleans, numbers, strings, lists, and dictionaries. Functions and namespaces cannot cross the process boundary. Keep callbacks pure when output ordering matters, because child output may interleave. An uncaught callback error propagates to the caller. On platforms without fork, `pmap` maps the elements serially.

### Reduce variables

```blood
var r = std.reduce

reduce(r.sum) var total = 0
reduce(r.max) var slowest_ms = 0
reduce(r.concat) var failures = []
reduce(r.or) var any_crash = false

reduce(r.sum) var (
    passed = 0
    failed = 0
)
```

`reduce(COMBINER) var ...` declares variables whose worker-local values are folded back into the parent when a parallel construct completes. The combiner is mandatory. `reduce var x = 0` is a syntax error. The initial value is mandatory because it is the snapshot every worker starts from.

The combiner is an ordinary expression that evaluates to a combiner value. Built-in combiners live in `std.reduce`.

| Combiner | Accepted values |
| --- | --- |
| `std.reduce.sum` | numbers |
| `std.reduce.product` | numbers; the initial value must be non-zero |
| `std.reduce.min` | numbers and strings |
| `std.reduce.max` | numbers and strings |
| `std.reduce.and` | booleans |
| `std.reduce.or` | booleans |
| `std.reduce.concat` | lists and strings |
| `std.reduce.union` | dictionaries; the right side wins on key conflicts |

A mismatched initial value, such as `sum` with a string, is an error at the declaration. `reduce` declarations may appear at file or function scope, but not inside a loop body.

A closure with exactly three explicit parameters is also a valid combiner.

```blood
reduce(|acc, snap, final| acc + (final - snap)) var total = 0
```

`acc` is the running merged value, `snap` is the parent's value at fork time, and `final` is the worker's value when it finished. The closure returns the new `acc`. The built-in combiners use the following rules.

| Combiner | Merge rule |
| --- | --- |
| `sum` | `acc + (final - snap)`. |
| `product` | `acc * final / snap`. |
| `min` and `max` | Compare `acc` with `final`. |
| `and` and `or` | Combine `acc` with `final`. |
| `concat` | Append only the part of `final` that extends `snap`. If `final` does not extend `snap`, the whole `final` value is appended. |
| `union` | Copy `acc`, then overlay every key from `final`. |

Workers start with `snap` as the variable's value. Reads and writes inside a worker act on the worker's private copy; workers never see each other's writes. A worker may reassign or compound-assign a reduce variable, including through methods such as `list.append`. Nested `pmap` calls merge into the enclosing worker's copy, which then merges upward.

Merging happens when `pmap` or `para for` completes, before it returns. `acc` starts as `snap`, and workers are folded in input order, not completion order, so the result is deterministic. The parent's value is unchanged between fork and join. If a worker fails, that worker's contribution is discarded; other workers still merge, and `pmap` still reports the error.

Outside a worker, a reduce variable behaves like a normal variable. Reduce variables provide reduction, not live shared state. There is no cross-worker visibility and no locking. The initial value and every worker's final value must be data values that can cross the process boundary. Blood currently has no `Time`, `Duration`, or set value types, so `min` and `max` accept numbers and strings, and `union` accepts dictionaries.

### Functions

```text
fn NAME(PARAM1, PARAM2: DEFAULT, ...) {
    BLOCK
}

fn NAME(PARAM1, PARAM2: DEFAULT, ...) => EXPRESSION
```

The `=>` form is shorthand for a body that immediately returns `EXPRESSION`. The parameter list may be omitted in either form when the function takes no arguments.

Parameters are positional unless passed by name. A parameter may declare a default after `:`, which is used when the caller omits that argument.

```blood
fn greet(name, sep: ", ") {
    return "Hello" + sep + name
}

greet("Ada", sep: ": ")
```

Type hints use Go-style syntax: the type follows the parameter name, and a return type follows the closing parenthesis. A default value follows `=`. For example, `fn add(a int, b int = 2) int`. The older `name: type` annotation form and `name: default` form remain accepted. Type information is metadata; it does not restrict runtime values.

A named function body uses the global environment as its parent, not the environment of its definition. Functions may be recursive. The recursion depth is limited to 200 nested user-function calls.

`return` with no value returns null. A function that reaches the end of its body returns null. Extra arguments are ignored; missing arguments without a default are null.

### Types and methods

A named type declares a set of fields. A field has a name, an optional type hint, and an optional default value.

```text
type NAME {
    FIELD TYPE [= DEFAULT]
}
```

A struct is constructed with `NAME{field: value, ...}`. Field names are required. Unknown fields, duplicate fields, and fields without a value or default raise a catchable error. The declared field type is metadata and is not enforced.

Methods are declared with a receiver type before the function name. A method is called with attribute syntax and receives the receiver as its first argument, conventionally named `self`.

```blood
type Result {
    file string
    pass bool
    ms   int = 0
}

fn Result.label(self) string => if self.pass { "PASS" } else { "FAIL" }

var r = Result{file: "a.sx", pass: true}
say("{r.label()} {r.file}")
```

Struct field reads and writes validate the field name. Reading or writing an unknown field raises a catchable error.

### Lambdas

```text
|l| EXPRESSION
|a, b| EXPRESSION
|a, b| => EXPRESSION
|a, b| {
    BLOCK
}
```

A lambda is an anonymous function value. The parameter list is enclosed by pipes, and the body is either a single expression or a braced block. An expression body is returned automatically; a block body returns when it reaches `return`, and otherwise returns null. Lambdas capture the environment in which they are defined, so they may refer to local variables of an enclosing function.

```blood
var lines = sh("test-suite").stdout.lines()
var passes = lines.count(|l| l.starts_with("[pass]"))
```

The `=>` form forces an expression body even when the expression begins with `{`. This makes dictionary literals usable directly.

```blood
var rows = files.map(|f| => {"file": f.basename(), "modified": f.modified()})
```

### Exceptions

```text
try {
    BLOCK
} catch NAME {
    BLOCK
}
```

`except` may be used in place of `catch`. The catch variable is optional and receives the error message as a string. A caught error clears the error state and execution continues after the `try` statement.

```text
raise EXPRESSION
throw EXPRESSION
```

Both spellings raise a runtime error whose message is the string representation of the expression. If the expression is omitted, the message is the string representation of null, namely `null`.

### `defer`

```text
defer EXPRESSION
defer {
    ...
}
```

Deferred statements run in last-in-first-out order when the enclosing function returns, when the script finishes, or when `exit()` is called. Pending defers also run while an uncaught runtime error unwinds. A deferred block runs in the environment where it was registered.

A single-expression defer is evaluated when it runs, not when it is registered. Function arguments are therefore read at defer time, and variables changed after the `defer` statement are seen in their final state. Use a block form when the timing of side effects matters.

### Other statements

The `pass` statement does nothing. `break` exits the innermost loop, and `continue` begins the next iteration of the innermost loop. Shell command statements execute the command line in the shell and assign the resulting status to `status`.

### Shell command statements

A statement that does not match a scripting statement is treated as a shell command when it has a command-like shape. The current implementation uses a lookahead heuristic. A leading identifier followed by an opening parenthesis, bracket, dot, or an assignment operator is an expression or assignment. A leading identifier followed by another identifier, a literal, or most operators is a shell command. A statement beginning with `/`, `.`, or `~` is a shell command.

Lines beginning with `if`, `while`, `for`, `para`, `fn`, `try`, `catch`, `except`, `raise`, `throw`, `say`, `return`, `break`, `continue`, `pass`, `defer`, `export`, `var`, `const`, `pub`, `use`, or `reduce` are scripting statements.

The entire remaining source line is passed to the shell command interpreter. This permits pipelines, redirections, and command substitution without a wrapper.

## Scoping and environment

Each function call creates a new environment. The global environment is the shared script state. A name lookup proceeds from the current environment through parent environments. Because function environments have the global environment as their parent, a function can read and update global variables but cannot capture local variables of a caller.

A binding created by `for` is assigned in the current environment and is visible after the loop.

Executing a script file as a command runs it in a fresh top-level environment. Its `var` and `fn` declarations do not leak into the interactive session, so the same script can be run repeatedly. `source` and startup files execute in the current environment instead, so their declarations remain visible.

The working directory is process-global, not scoped. `cd(path)` calls `chdir` in the interpreter process, updates `PWD`, and affects all later shell commands, `run` calls, and file operations. There is no automatic restoration on function return.

## Error handling and diagnostics

Syntax errors are detected before execution. A syntax error prints the following form, where `LABEL` is the script path, `stdin`, or `-c`, and execution of the script aborts with status 2.

```text
blood: LABEL:LINE:COL: MESSAGE
```

Runtime errors print the same form and abort the script with status 1 unless caught by `try`. An interactive script interrupted by SIGINT is reported as status 130 without printing the message `interrupted`.

| Message | Condition |
| --- | --- |
| `undefined variable 'NAME'` | A name is not bound in the environment chain. |
| `attempt to call a non-function value` | A call target is not callable. |
| `list index out of range` | A list index is outside the valid range. |
| `string index out of range` | A string index is outside the valid range. |
| `value is not indexable` | An index is applied to a value that does not support indexing. |
| `method call on null` | A method is called on a null value. |
| `unknown member 'NAME'` | An attribute is not defined on the receiver. |
| `unknown field 'NAME'` | A struct literal names a field that is not declared. |
| `missing field 'NAME'` | A struct literal omits a field that has no default. |
| `value does not support attribute assignment` | An attribute assignment targets an unsupported value. |
| `division by zero` | Division or remainder by zero is attempted. |
| `unknown method 'NAME'` | A method name is not defined for the receiver. |
| `recursion limit exceeded` | User-function recursion exceeds 200 calls. |
| `interrupted` | Execution was interrupted by SIGINT. |

## Standard variables

| Name | Meaning |
| --- | --- |
| `status` | Status of the most recent shell command, command substitution, or shell-valued operation in the script. Initialised to 0. |
| `args` | List of command-line arguments passed after the script name. |
| `__file__` | Path of the currently executing script, set by script file execution. |
| `null` | The null value. |
| `true`, `false` | Boolean values. |

`status` is also updated after a command substitution and after a shell command statement. A failed shell command does not raise a script error; it sets `status`.

`status` is deprecated for new scripts. `sh` and `run` return a result dictionary with `status`, `stdout`, `stderr`, and `ok` fields; use those fields instead. `status` remains available while existing scripts are migrated.

## Builtin functions

Builtins are grouped in the `std` namespace. They are ordinary function values and can be called as `std.len(x)`, `std.read_file(path)`, and so on.

A small bare-name prelude is also available: the `say` statement and the functions `print`, `eprint`, `exit`, `cd`, `run`, `len`, `str`, `range`, and `sh`. These names may be shadowed by user definitions; a warning is written to standard error when that happens. The `std.` forms are always available and never shadowed, so `std.print(...)`, `std.len(...)`, and `std.sh(...)` are the unambiguous spellings.

The `std.os` value is the host platform name reported by the build: `darwin` on macOS and `linux` on other targets.

```blood
if std.os == "darwin" {
    say "hi macos"
}
```

| Function | Arity | Behaviour |
| --- | --- | --- |
| `std.say(...)` | variadic | Join arguments with spaces, append newline, write to standard output. Equivalent to the `say` statement. Returns null. |
| `std.len(x)` | 1 | Length of a list, dictionary, or string. String length is bytes. For other values, length of the string representation. |
| `std.str(x)` | 1 | String representation. |
| `std.int(x)` | 1 | Convert to integer. |
| `std.float(x)` | 1 | Convert to float. |
| `std.bool(x)` | 1 | Truthiness as a boolean. |
| `std.range(a)` / `std.range(a,b)` | 1 or 2 | List of integers from `a` (inclusive) to `b` (exclusive). With one argument, starts at 0. |
| `std.print(...)` | variadic | Join arguments with spaces, append newline, write to standard output. Returns null. |
| `std.input([prompt])` | 0 or 1 | Write prompt if supplied, read one line from standard input. Returns a string, or null on end of input. |
| `std.sleep(ms)` | 1 | Sleep for the given number of milliseconds. |
| `std.exit([code])` | 0 or 1 | Request shell exit with the given status. Default 0. |
| `sh(command, check: bool)` | 1 | Execute a command line and return a result dictionary with `status`, `stdout`, `stderr`, and `ok`. Both output streams are captured. With `check: true`, a non-zero status raises a runtime error. |
| `std.run(argv, merge_stderr: bool, cwd: string)` | 1 | Execute a list of arguments directly without a shell and return a result dictionary. `merge_stderr` combines stderr into `stdout`; `cwd` sets the child working directory. |
| `std.eprint(...)` | variadic | Like `print`, but writes to standard error. |
| `std.indent(text, n)` | 2 | Prefix every line in `text` with `n` spaces. |
| `std.exists(path)` | 1 | Whether a path exists. |
| `std.is_file(path)` | 1 | Whether a path exists and is not a directory. |
| `std.is_dir(path)` | 1 | Whether a path is a directory. |
| `std.is_executable(path)` | 1 | Whether a path is executable. |
| `std.which(name)` | 1 | Search `PATH` and return the executable path, or null. |
| `std.mktemp_dir(prefix)` | 1 | Create and return a unique temporary directory. |
| `std.mkdir_p(path)` | 1 | Create a directory and any missing parent directories. Returns a boolean. |
| `std.remove_file(path)` | 1 | Remove one file. Returns a boolean. |
| `std.remove_all(path_or_list)` | 1 | Recursively remove paths. Returns a boolean. |
| `std.parent(path)` | 1 | Directory component of a path. |
| `std.basename(path)` | 1 | Final component of a path. |
| `std.stem(path)` | 1 | Final component with its last extension removed. |
| `std.join(a, b)` | 2 | Join two path components. |
| `std.cd(path)` | 1 | Change the interpreter working directory. Affects later shell and `run` calls and file operations. |
| `std.unified_diff(expected, actual)` | 2 | Line-oriented diagnostic diff for test harnesses. |
| `std.source(path)` | 1 | Execute a Blood script file and return its status. |
| `std.this()` | 0 | Name and absolute path of the executing script as a dictionary with keys `name` (the final path component) and `path` (absolute). Returns null when no script file is executing. |
| `std.this_dir()` | 0 | Absolute path of the directory containing the executing script. Returns null when no script file is executing. |
| `std.abs(x)` | 1 | Absolute numeric value, returned as a float. |
| `std.min(x)` / `std.min(a,b)` | 1 or 2 | Minimum of a list, or of two values. With a list, returns the smallest element. |
| `std.max(x)` / `std.max(a,b)` | 1 or 2 | Maximum of a list, or of two values. |
| `std.sum(list)` | 1 | Sum of numeric elements, returned as a float. |
| `std.type(x)` | 1 | Type name: `null`, `bool`, `int`, `float`, `string`, `list`, `dict`, `function`, or `namespace`. |
| `std.keys(dict)` | 1 | List of dictionary keys. |
| `std.values(dict)` | 1 | List of dictionary values. |
| `std.items(dict)` | 1 | List of `[key, value]` pairs. |
| `std.glob(pattern)` | 1 | Expand a pathname pattern and return a sorted list of matching paths. An unmatched pattern yields an empty list. A leading `~` is expanded. |
| `std.rename(from, to)` | 2 | Rename or move a path. Returns a boolean. A leading `~` is expanded in both paths. |
| `std.read_file(path, default: x)` | 1 | Read a file. Returns a string, or `x` when `default:` is given and the file cannot be read. Without a default, returns null. |
| `std.write_file(path, data)` | 2 | Write a file. Returns a boolean. |
| `std.append_file(path, data)` | 2 | Append to a file. Returns a boolean. |
| `std.env(name, default: x)` | 1 | Read an environment variable. Returns `x` when `default:` is given and the variable is unset or empty; otherwise returns a string or null. |
| `std.clock()` | 0 | Milliseconds since the Unix epoch. |
| `std.chr(n)` | 1 | One-byte string containing the low byte of `n`. |
| `std.ord(s)` | 1 | Integer value of the first byte of `s`, or 0 for the empty string. |
| `std.set_workers(n)` | 1 | Set the maximum number of concurrent workers for `para for` and `pmap`. Returns the effective limit. A value above the number of logical processors is reduced to that number with a warning. |

`std.keys`, `std.values`, and `std.items` return empty lists when applied to a non-dict value. `std.read_file` and `std.write_file` expand a leading `~` in the path.

The default worker limit is the number of logical processors. `std.set_workers` affects subsequent parallel constructs and returns the limit that was applied.

## Methods

Method calls use the attribute syntax `value.name(arguments)`.

### List methods

| Method | Arity | Behaviour |
| --- | --- | --- |
| `append(x)` | 1 | Append x. Returns null. |
| `pop()` | 0 | Remove and return the last element, or null when empty. |
| `last()` | 0 | Return the last element, or null when empty. |
| `len()` | 0 | Number of elements. |
| `insert(i,x)` | 2 | Insert x at index i. |
| `remove(i)` | 1 | Remove the element at index i. |
| `contains(x)` | 1 | Membership by equality. |
| `join(sep)` | 1 | Join string representations with separator `sep`. |
| `sort()` | 0 | Sort in place by string representation. |
| `sort_by(fn)` | 1 | Sort in place by the key returned by `fn(x)`. Numeric keys compare numerically. Returns the list. |
| `reverse()` | 0 | Reverse the list in place. Returns the list. |
| `unique()` | 0 | New list with duplicate elements removed, preserving first-occurrence order. Lists and dictionaries compare by contents. |
| `dump()` | 0 | Pretty-printed JSON representation. |
| `dump_compact()` | 0 | Compact JSON representation. |
| `map(fn)` | 1 | New list containing `fn(x)` for each element `x`. |
| `pmap(fn)` | 1 | Like `map(fn)`, but calls run concurrently in child processes. Results keep the original input order. |
| `filter(fn)` | 1 | New list of elements for which `fn(x)` is truthy. |
| `count([fn])` | 0 or 1 | Number of elements. With `fn`, the number of elements for which `fn(x)` is truthy. |

### Dictionary methods

| Method | Arity | Behaviour |
| --- | --- | --- |
| `get(key[, default: x])` | 1 or 2 | Value for key, or the supplied default/null. |
| `set(key, value)` | 2 | Set a key. |
| `contains(key)` | 1 | Key membership. |
| `keys()` | 0 | List of keys. |
| `sorted()` / `sorted_keys()` | 0 | New list of keys sorted by string value. |
| `values()` | 0 | List of values. |
| `items()` | 0 | List of `[key, value]` pairs. |
| `dump()` | 0 | Pretty-printed JSON representation. |
| `dump_compact()` | 0 | Compact JSON representation. |
| `len()` | 0 | Number of keys. |
| `remove(key)` | 1 | Remove a key. |

Any other method call on a dictionary inserts the first argument as the value for the method name and returns null. This is an implementation quirk; named keys should normally be accessed with indexing or assignment.

### String methods

| Method | Arity | Behaviour |
| --- | --- | --- |
| `upper()` | 0 | ASCII uppercase. |
| `lower()` | 0 | ASCII lowercase. |
| `strip()` | 0 | Remove leading and trailing ASCII whitespace. |
| `split(sep)` | 1 | Split on `sep`. An empty separator splits UTF-8 code points. |
| `lines()` | 0 | Split into lines. A trailing newline does not add an empty final element; `\r\n` is accepted. |
| `split_words()` | 0 | Split on whitespace, honouring single and double quotes. |
| `startswith(prefix)` / `starts_with(prefix)` | 1 | Prefix test. |
| `endswith(suffix)` / `ends_with(suffix)` | 1 | Suffix test. |
| `strip_suffix(suffix)` | 1 | Remove one trailing suffix when present. |
| `contains(needle)` | 1 | Substring test. |
| `matches(pattern)` | 1 | Regular-expression search. |
| `like(pattern, ignore_case: BOOL)` | 1 | Glob match. `*` matches any sequence and `?` matches one byte. `ignore_case` is an optional named argument. |
| `jtable()` | 0 | Parse the receiver as JSON and render an array of objects, or one object, as a Unicode table. |
| `replace(from,to)` | 2 | Replace all occurrences. |
| `last()` | 0 | Return the last UTF-8 code point, or null when empty. |
| `len()` | 0 | Byte length. |
| `chars()` | 0 | List of UTF-8 code points as strings. |
| `join(list)` | 1 | Join a list with the receiver as separator. |
| `parent()` | 0 | Directory component of a path. |
| `basename()` | 0 | Final component of a path. |
| `stem()` | 0 | Final component of a path with its last extension removed. |
| `modified()` | 0 | File modification time as seconds since the Unix epoch, or `0` when unavailable. |
| `enc_b64()` | 0 | Base64-encode the string. |
| `dec_b64()` | 0 | Decode the string as base64. Invalid input raises a catchable error. |
| `file_enc_b64()` | 0 | Read a file and return its contents as base64. |
| `file_dec_b64()` | 0 | Read a base64 file and return the decoded contents. Invalid input raises a catchable error. |
| `abs()` | 0 | Absolute, cleaned path. Expands `~` and resolves relative paths against the current directory. |
| `exists()` | 0 | Whether the path exists. |
| `is_file()` | 0 | Whether the path exists and is not a directory. |
| `is_dir()` | 0 | Whether the path is a directory. |
| `is_executable()` | 0 | Whether the path is executable. |

`jtable()` accepts a JSON array of objects and returns a Unicode table whose columns are the sorted object keys. A single JSON object becomes a one-row table. Nested arrays and objects are shown as `[...]` and `{...}`.

### Numeric methods

Integers and floats provide `abs()`, returning a float.

## Script grammar

The following grammar describes the scripting language accepted by the implementation. `NEWLINE` and `EOF` are produced by the lexer as described above. `SHELL` is a statement produced by the heuristic described in [Shell command statements](#shell-command-statements). `STRING` is a string literal, including interpolation text. `CMDSUB` is a command substitution token.

### Lexical grammar

```text
letter       = "A".."Z" | "a".."z" ;
digit        = "0".."9" ;
IDENT        = ( letter | "_" ) { letter | digit | "_" } ;
INT          = digit { digit } ;
FLOAT        = digit { digit }
               ( "." digit { digit } [ exponent ] | exponent ) ;
exponent     = ( "e" | "E" ) [ "+" | "-" ] digit { digit } ;
STRING       = "'" { string-byte } "'"
             | '"' { string-byte } '"'
             | "'''" { any-byte } "'''"
             | '"""' { any-byte } '"""' ;
RAWSTRING    = ( "r'" { any-byte } "'" | 'r"' { any-byte } '"' ) ;
CMDSUB       = "`" { any-byte } "`"
             | "$(" { any-byte } ")" ;
```

The lexer emits `NEWLINE` and `EOF`. Indentation carries no meaning. Newlines are ignored while the delimiter depth of parentheses or brackets is greater than zero, and newlines separate statements inside braces.

### Syntactic grammar

```text
program      = { NEWLINE | statement } EOF ;

statement    = say-stmt
             | return-stmt
             | break-stmt
             | continue-stmt
             | pass-stmt
             | raise-stmt
             | if-stmt
             | while-stmt
             | for-stmt
             | para-for-stmt
             | fn-stmt
             | type-stmt
             | try-stmt
             | var-stmt
             | const-stmt
             | assignment
             | export-stmt
             | defer-stmt
             | reduce-stmt
             | use-stmt
             | pub-stmt
             | expression-stmt
             | SHELL ;

suite        = "{" { NEWLINE | ";" | statement } "}" ;

inline-stmt  = say-stmt | return-stmt | break-stmt | continue-stmt
             | pass-stmt | raise-stmt | var-stmt | assignment | export-stmt
             | defer-stmt | expression-stmt ;

say-stmt     = "say" [ expression { "," expression } ] [ NEWLINE ] ;
return-stmt  = "return" [ expression ] [ NEWLINE ] ;
break-stmt   = "break" [ NEWLINE ] ;
continue-stmt= "continue" [ NEWLINE ] ;
pass-stmt    = "pass" [ NEWLINE ] ;
raise-stmt   = ( "raise" | "throw" ) [ expression ] [ NEWLINE ] ;

if-stmt      = if-expr ;

if-expr      = "if" expression suite
               { "elif" expression suite }
               [ "else" suite ] ;

while-stmt   = "while" expression suite ;

for-stmt     = "for" IDENT [ "," IDENT ] "in" expression suite ;

para-for-stmt = "para" "for" IDENT "in" expression suite ;

fn-stmt      = "fn" IDENT [ "." IDENT ] [ "(" [ param { "," param } [ "," ] ] ")" ]
               [ type-text ] ( suite | "=>" expression ) ;
param        = IDENT [ type-text ] [ "=" expression ] ;
type-stmt    = "type" IDENT "{" { NEWLINE | type-field } "}" ;
type-field   = IDENT [ type-text ] [ "=" expression ] ;

try-stmt     = "try" suite ( "catch" | "except" ) [ IDENT ] suite ;

var-stmt     = "var" ( var-item
                     | "(" { NEWLINE | "," | ";" | var-item } ")"
                     ) [ NEWLINE ] ;
var-item     = IDENT "=" expression ;
const-stmt   = "const" IDENT "=" expression [ NEWLINE ] ;
assignment   = expression assign-op expression [ NEWLINE ] ;
export-stmt  = "export" IDENT assign-op expression [ NEWLINE ] ;
defer-stmt   = "defer" expression [ NEWLINE ] | "defer" suite ;
assign-op    = "=" | "+=" | "-=" | "*=" | "/=" | "%=" ;

reduce-stmt  = "reduce" "(" expression ")" "var"
               ( reduce-item
               | "(" { NEWLINE | "," | ";" | reduce-item } ")" ) ;
reduce-item  = IDENT "=" expression ;

use-stmt     = "use" STRING [ NEWLINE ] ;
pub-stmt     = "pub" ( var-stmt | const-stmt | fn-stmt | type-stmt ) ;

expression-stmt = expression [ NEWLINE ] ;

expression   = coalesce-expr ;
coalesce-expr = or-expr [ "??" coalesce-expr ] ;
or-expr      = and-expr { "or" and-expr } ;
and-expr     = not-expr { "and" not-expr } ;
not-expr     = "not" not-expr | comparison ;
comparison   = range-expr
               [ ( "==" | "!=" | "<" | ">" | "<=" | ">="
                 | "in" | "not" "in" ) range-expr ] ;
range-expr   = additive [ ( ".." | "..=" ) additive ] ;
additive     = multiplicative { ( "+" | "-" ) multiplicative } ;
multiplicative = unary { ( "*" | "/" | "%" ) unary } ;
unary        = ( "-" | "+" ) unary | postfix ;
postfix      = primary { call | index | attribute } ;
call         = "(" [ argument { "," argument } [ "," ] ] ")" ;
argument     = [ IDENT ":" ] expression ;
index        = "[" expression "]" ;
attribute    = "." IDENT ;

primary      = INT
             | FLOAT
             | STRING
             | RAWSTRING
             | CMDSUB
             | "true"
             | "false"
             | "null"
             | "none"
             | IDENT
             | "(" expression ")"
             | list-literal
             | dict-literal
             | if-expr
             | lambda ;

list-literal = "[" [ list-item { "," list-item } [ "," ] ] "]" ;
list-item    = [ "..." ] expression ;
dict-literal = "{" [ expression ":" expression
                     { "," expression ":" expression } [ "," ] ] "}" ;
lambda       = "|" [ IDENT { "," IDENT } ] "|"
               ( "=>" expression | expression | "{" { statement } "}" ) ;
```

`type-text` is accepted and stored for tooling; type annotations have no runtime effect. A parameter default follows `=` in the Go-style form, or `:` for compatibility with older scripts. A `SHELL` statement is recognised by the statement-start heuristic and consumes the remainder of the source line.

## Evaluation order

Expressions are evaluated left to right except where short-circuit operators skip the right operand. Function arguments are evaluated left to right. Dictionary literal entries are evaluated in source order; iteration order after construction is unspecified.

A runtime error sets the error state and stops normal evaluation. `try` catches the error at the nearest enclosing `try` statement. A `return`, `break`, `continue`, or `exit` is not an error and is not caught by `try`.

## Limits

| Limit | Value |
| --- | --- |
| Maximum user-function recursion depth | 200 |
| Parallel worker limit | Number of logical processors by default; changeable with `std.set_workers` |
| History maximum | Controlled by configuration |
| Integer arithmetic | Signed 64-bit and not overflow checked |
| Floating-point arithmetic | Platform double implementation |
| String representation | Byte strings; UTF-8 is handled specially only by code-point iteration, string indexing boundaries, and the line editor |
