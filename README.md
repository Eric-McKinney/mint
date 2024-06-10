<h1 align="center"> mint </h1>

<p align="center">
    <img src="https://github.com/Eric-McKinney/mint/actions/workflows/build.yml/badge.svg">
</p>

## Description

Currently WIP. A math interpreter. Use either via command line or write a script.

## Functionality

The idea is that mint will be able to interpret (and evaluate) any valid mathematical expression either in an
interactive command line session or by being invoked on a script. I also plan to allow for loading/including scripts
into the command line session to have access to variables and functions defined in that environment. This should also
work for scripts, but I'm not sure yet how I'll avoid circular imports.

Inputs look something like this:

```
x = 3
y = 4

// a comment (must occur on its own line)

z = x + y
z2 = x+y

// x and y are shadowed
fn f(x, y) = x^(y+3)

// z is used from outer scope
fn f2(a) = 2*a - z

// this also updates the z used by f2
z = 22

f(4,1)
f(1,1+1) * (2 + 1)
```

Right now I'm focusing on covering int/float arithmetic and function definitions. I will implement more functionality
after. I plan on adding built in functions (sqrt, factorial, etc.), maybe sums and products, and relevant constants
like pi and e. If I'm feeling adventurous I might try to implement complex numbers.

## Implementation

For the lexer I'm thinking have some struct for a token w/enum field (token type), 
optional value field, and then a pointer field to the next token (it's going to be a linked list)

The language should follow math associativity and precedence rules (obviously)

## CFG

Expr -> FunctionExpr | AssignmentExpr | AdditiveExpr `\n`\
FunctionExpr -> `fn` ID `(` ParamExpr `)` `=` AdditiveExpr `\n`\
ParamExpr -> ID`,` ParamExpr | ID\
AssignmentExpr -> ID `=` AdditiveExpr `\n`\
AdditiveExpr -> AdditiveExpr AdditiveOperator MultiplicativeExpr | MultiplicativeExpr\
AdditiveOperator -> `+` | `-`\
MultiplicativeExpr -> MultiplicativeExpr MultiplicativeOperator ApplicationExpr | ApplicationExpr\
MultiplicativeOperator -> `*` | `/`\
ApplicationExpr -> ID`(`ArgExpr`)` | PrimaryExpr\
ArgExpr -> AdditiveExpr`,` ArgExpr | AdditiveExpr\
PrimaryExpr -> `int` | `float` | ID | `(`AdditiveExpr`)`\
ID -> `string which matches the following regex: ^[a-zA-Z][a-zA-Z0-9_]*$`

# TODO

- [x] ~~Write lexer~~
- [x] ~~Write parser~~
- [x] ~~Write evaluator~~
    - [ ] Lots of testing
- [ ] Write main cli/script-running loop
    - [ ] Lots of testing
- [ ] Document code
    - [ ] Describe less obvious functions w/comments (mostly in eval)
    - [ ] Put relevant CFG bits before parser functions
- [ ] Clean up README & add screenshots/gifs using mint

## Longer term TODO

- [ ] Explore parsing arithmetic using the [shunting yard algorithm](https://www.engr.mun.ca/~theo/Misc/exp_parsing.htm#shunting_yard)
or the [precedence climbing algorithm](https://www.engr.mun.ca/~theo/Misc/exp_parsing.htm#climbing)
- [ ] Add multiplication via (45)(3) syntax (change CFG too)
- [ ] Add exponents
- [ ] Add trig functions (radians only)
    - [ ] Add deg\_to\_rad function
    - [ ] Add rad\_to\_deg function
- [ ] Start adding version numbers, whatnot
- [ ] Maybe add to package managers like homebrew?

