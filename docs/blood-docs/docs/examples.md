# Examples

## Interactive shell

```text
$ blood
╭─ blood v0.1.5 — type help for commands, help script for the language.
┌─ user@host ~/src (main *)
└─ ❯ echo hello | tr a-z A-Z
HELLO
┌─ user@host ~/src (main *)
└─ ❯ for item in ["one", "two"] {
  …     say item
  … }
```

## Command language

```text
# Pipelines
ls -l | grep '\.bx$'

# Redirection
echo hello > out.txt
cat out.txt >> all.txt
command 2> errors.txt

# Command substitution
echo "time is $(date +%H:%M)"

# Background execution
sleep 5 &
jobs

# Environment assignment
EDITOR=vim command
export EDITOR=vim
unset EDITOR
```

## Scripting language

```blood
var name = "Blood"
var version = 1

say "Welcome to {name} {version}"

for item in ["one", "two", "three"] {
    say item
}

fn greet(person) {
    return "Hello, " + person
}

say greet("world")

if version > 0 {
    say "positive"
} elif version == 0 {
    say "zero"
} else {
    say "negative"
}

try {
    raise "example failure"
} catch message {
    say "caught: {message}"
}

var output = `printf "a\nb\n"`
say output
say "status is {status}"

sh("true")
```
