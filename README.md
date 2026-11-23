# blood

Shell built for performance, reliability and ergonomics.

Written with the [Spectre Programming Language](https://spectrelang.org)

Outperforms ksh and dash on several benchmarks, outperforming Elvish and Fish by up to 120x on general purpose tasks as per [Shell Arena](https://navid-m.github.io/shellarena/).

```h
for x in ["hello", "world"] {
    sh("cowsay " + x)
}
```

And another example:

```h
cd("./src/lang")
say(std.glob("*.sx").map(|f| => {"file": f.basename(), "mod": f.modified()}).dump().jtable())
```

Output:

```
┌──────────────┬────────────┐
│ file         │ mod        │
├──────────────┼────────────┤
│ format.sx    │ 1795343823 │
│ lang.sx      │ 1795346879 │
│ lsp.sx       │ 1795343822 │
│ script.sx    │ 1795356865 │
│ translate.sx │ 1795214737 │
└──────────────┴────────────┘
```

Yet another example:

```h
say("hello".enc_b64())     # aGVsbG8=
say("aGVsbG8=".dec_b64())  # hello
```

#### Quick install

POSIX:

```shell
git clone https://github.com/navid-m/blood.git && cd blood/fw
./install.sh
```

Windows:

```powershell
git clone https://github.com/navid-m/blood.git
cd .\blood\fw
.\install.ps1
```

## Resources

- [Site](https://bloodshell.pages.dev)
- [Specification](https://bloodshell.pages.dev/specification.html)
- [Benchmark Results](https://navid-m.github.io/shellarena/)
- [Discord](https://discord.gg/sPS9xQc92X)

---

&copy; Navid Momtahen - 2026

License: GPL-3.0-only
