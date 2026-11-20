# blood

Shell built for performance, reliability and ergonomics.

Written with the [Spectre Programming Language](https://spectrelang.org)

Outperforms ksh and dash on several benchmarks, outperforming Elvish and Fish by up to 120x on general purpose tasks as per [Shell Arena](https://navid-m.github.io/shellarena/).

```
for x in ["hello", "world"] {
    sh("cowsay " + x)
}
```

Quick install:

```shell
git clone https://github.com/navid-m/blood.git && cd blood
./fw/build.sh && sudo mv blood /usr/bin/blood
```

Builtins live under the `std` namespace, such as `std.read_file` and
`std.glob`. Only `say`, `print`, `eprint`, `exit`, `cd`, `run`, `len`, `str`,
`range`, and `sh` are available as short bare-name prelude calls; their `std.`
forms always work too.

## Resources

- [Site](https://bloodshell.pages.dev)
- [Specification](https://bloodshell.pages.dev/specification.html)
- [Benchmark Results](https://navid-m.github.io/shellarena/)
- [Discord](https://discord.gg/sPS9xQc92X)

## Examples

The shell and the language are one. shell commands are values, strings interpolate, and loops hold real data. Thus, there are no quoting layers to get tangled with.

```
fn lines(path) {
    return std.len(std.read_file(path).strip().split("\n"))
}

var total = 0

for file in std.glob("src/*.sx") {
    var n = lines(file)
    total += n
    say "{file}: {n} lines"
}

say "source total: {total} lines"
```

Note that captured commands are ordinary strings, so you can do:

```
branch = sh("git branch --show-current").strip()
say "working on {branch}"

changes = sh("git status --porcelain").strip()
if changes != "" {
    for change in changes.split("\n") {
        say "  {change}"
    }
}
```

Parallelism is a keyword, therefore every iteration is a child process. No thread pools or async/await.

```
para for file in std.glob("src/*.sx") {
    say sh("sha256sum {file}").strip()
}
```

## Translating bash scripts

Translate one script, or a directory tree of scripts, to Blood:

```
blood -translate-sh script.sh
blood -translate-sh -o translated scripts/
```

Files are written next to their source by default, with `.sh` or `.bash`
replaced by `.bx`. Directories are walked recursively.

---

&copy; Navid Momtahen - 2026

License: GPL-3.0-only
