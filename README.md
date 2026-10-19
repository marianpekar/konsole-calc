# konsole-calc

When I switched from Windows back to Linux (CachyOS) after many years, with Konsole and fish I missed only one thing I was used to in PowerShell and that is how it evaluates arithmetic expressions.

Though I found some solutions like `echo $((3+3))` (doesn't work with fish btw) or just using `math` (see the links below), I then realized I know how to implement a tokenizer and parser to build an AST and evaluate it. I've done it before in C# for a fully functional VR calculator in Unity, and once in Java while going through great book *Crafting Interpreters* by Robert Nystrom. I thought it would be fun to do it again in C, this time with integration scripts so I can install and remove it easily and use it in Konsole like this:

```sh
❯(2^-2*4)+2
3
~
```

Or with variables:

```sh
❯ set x 4

~
❯ = 5+$x
9
```

The tricky part is shell syntax, not arithmetic. Fish expands globs and parentheses before a command ever sees its arguments, so a bare `2*3` fails with `No matches for wildcard...`. The first working version required an `=` prefix and quotes around the expression.

To remove that friction, `integration/konsole-calc.fish` binds Enter to a function that inspects the command line before fish parses it. If the line starts with `=`, it strips the `=` and any following whitespace. Otherwise, a regex checks whether the line looks like an arithmetic expression. In either case, the line is rewritten to `= "<expression>"` and executed, so globs and parentheses are never expanded. This gives me the original PowerShell experience.

Variable expansion is handled by fish.

## wtFAQ

**Is this solution to a simple problem an absolute overkill?** 
- Yes, it is.

## Links

- https://stackoverflow.com/questions/2552130/bash-evaluate-a-mathematical-term
- https://fishshell.com/docs/current/cmds/math.html
- https://craftinginterpreters.com/
- https://cachyos.org/