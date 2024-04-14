# mint
A math interpreter.

## Functionality
So the idea is that the interpreter can interpret files or stdin


Inputs look something like this:

```
x = 3;

y = 4;

z = x + y;

fn f(x, y) = x^(y+3);

f(4,1);
```
(maybe try without semicolon?)


I'm thinking bare minimum I cover int/float arithmetic and function definitions. Built in functions can come later.

## Implementation
For the lexer I'm thinking have some struct for a token w/enum field (token type), 
optional value field, and then a pointer field to the next token (it's going to be a linked list)


The language should be follow math associativity and precedence rules
