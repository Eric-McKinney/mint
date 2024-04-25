# mint
Currently WIP. A math interpreter. Use either via command line or write a script.

## Functionality
So the idea is that the interpreter can interpret files or stdin. I'm thinking actually there's little point to just interpreting a file in a vacuum. It would be more useful to be able to interpret a file and load the environment, results, etc. into the command line interpreter session.


Inputs look something like this:

```
x = 3
y = 4

z = x + y

fn f(x, y) = x^(y+3)

f(4,1)
f(1,1+1)!
```

I'm thinking bare minimum I cover int/float arithmetic and function definitions. Built in functions can come later. I want to cover things like factorial, maybe sums and products, and relevant constants like pi and e. If I'm feeling adventurous I might try to implement complex numbers.

## Implementation
For the lexer I'm thinking have some struct for a token w/enum field (token type), 
optional value field, and then a pointer field to the next token (it's going to be a linked list)


The language should follow math associativity and precedence rules (obviously)

## CFG
Expr -> FunctionExpr | AssignmentExpr | AdditiveExpr\
FunctionExpr -> `fn` ID `(` ParamExpr `)` `=` AdditiveExpr\
ParamExpr -> ID`,` ParamExpr | ID\
AssignmentExpr -> ID `=` AdditiveExpr\
AdditiveExpr -> AdditiveExpr AdditiveOperator MultiplicativeExpr | MultiplicativeExpr\
AdditiveOperator -> `+` | `-`\
MultiplicativeExpr -> MultiplicativeExpr MultiplicativeOperator ApplicationExpr | ApplicationExpr\
MultiplicativeOperator -> `*` | `/`\
ApplicationExpr -> ID`(`ArgExpr`)` | PrimaryExpr\
ArgExpr -> AdditiveExpr`,` ArgExpr | AdditiveExpr\
PrimaryExpr -> `int` | `float` | ID | `(`AdditiveExpr`)`\
ID -> `string which matches the following regex: ^[a-zA-Z][a-zA-Z0-9_]*$`

# TODO
- [ ] Write lexer
    - [x] ~~Define token type~~
    - [x] ~~Determine list of all valid tokens~~ (might add more later)
    - [x] ~~Write tokenize function~~
    - [x] ~~Put types and function prototypes in a header file~~
    - [ ] Do lots of testing
- [ ] Write parser
    - [ ] Verify that CFG is correct (aligns with math rules)
    - [ ] Write various parse functions for each level of CFG
- [ ] Write evaluator
- [ ] Lots of testing
- [x] ~~Move makefile to project root and update accordingly~~
    - [ ] Write rules to build tests
