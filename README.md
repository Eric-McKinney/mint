<h1 align="center"> mint </h1>

<p align="center">
    <img src="https://github.com/Eric-McKinney/mint/actions/workflows/build.yml/badge.svg">
</p>

## Description

A math interpreter which supports variables and functions. Use either via command line or write a script.

## Functionality

Mint can interpret (and evaluate) any valid arithmetic expression either in an interactive command line session or by being invoked on a script.

See the [wiki](https://github.com/Eric-McKinney/mint/wiki) for more detailed explanations and examples, but anyways here's a brief example of what you can do:

```
mint|> 1 + 2 + 3 + 4
10
mint|> pi = 3.14
pi: 3.140000
mint|> r = 5
r: 5
mint|> pi * r * r
78.500000
mint|> fn area(r) = pi*r*r
area(r) = pi * r * r
mint|> area(5)
78.500000
mint|> area(1)
3.140000
mint|> r
r: 5
```

## CFG

Input -> Expr `\n` | Comment `\n`\
Comment -> Expr `#` `any text` | `#` `any text`\
Expr -> FunctionExpr | AssignmentExpr | AdditiveExpr\
FunctionExpr -> `fn` ID `(` ParamExpr `)` `=` AdditiveExpr\
ParamExpr -> ID`,` ParamExpr | ID\
AssignmentExpr -> ID `=` AdditiveExpr\
AdditiveExpr -> AdditiveExpr AdditiveOperator MultiplicativeExpr | MultiplicativeExpr\
AdditiveOperator -> `+` | `-`\
MultiplicativeExpr -> MultiplicativeExpr MultiplicativeOperator ExponentExpr | ExponentExpr\
MultiplicativeOperator -> `*` | `/`\
ExponentExpr -> ApplicationExpr `^` ApplicationExpr | ApplicationExpr\
ApplicationExpr -> ID`(`ArgExpr`)` | PrimaryExpr\
ArgExpr -> AdditiveExpr`,` ArgExpr | AdditiveExpr\
PrimaryExpr -> `int` | `float` | ID | `(`AdditiveExpr`)`\
ID -> `string which matches the following regex: ^[a-zA-Z][a-zA-Z0-9_]*$`

## TODO

- [ ] Explore parsing arithmetic using the [shunting yard algorithm](https://www.engr.mun.ca/~theo/Misc/exp_parsing.htm#shunting_yard)
or the [precedence climbing algorithm](https://www.engr.mun.ca/~theo/Misc/exp_parsing.htm#climbing)
- [ ] Add multiplication via (45)(3) syntax (change CFG too)
- [ ] Add exponents
- [ ] Add builtin functions like sqrt, factorial, sums, products, etc.
- [ ] Add builtin constants like e and pi
- [ ] Add trig functions (radians only)
    - [ ] Add deg\_to\_rad function
    - [ ] Add rad\_to\_deg function
- [ ] Start adding version numbers, whatnot
- [ ] Maybe add to package managers like homebrew/aur/nix?
- [ ] Add ability to import scripts into mint shell session
- [ ] Add ability to import scripts in other scripts
    - [ ] Avoid circular imports
- [ ] Add curses based tui which shows defined variables and their values in real time alongside mint shell

