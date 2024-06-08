# Log

### Preamble

I'm starting this log to keep myself focused, track my progress, and to be able to reference it later. \
The format for each entry is as follows:
- A goals section describing what I intend to get done (checked off as I finish each thing)
- A notes section for any significant unplanned changes I make or just to say everything went to plan
- An optional issues section where I describe problems I'm having
- An optional fixes section where I describe the solutions/fixes to the problems in the issues section

I wish I thought of this earlier, but it is what it is.

---

### June 1st, 2024

Goals for today:
- [x] Implement pop\_params
- [x] Implement bind\_args
- [x] Implement eval\_arguments
- [x] Ensure that eval.c compiles without any warnings
- [x] Begin writing tests for eval.c

Notes:
- Added validate\_params function to check for duplicate parameters
    - Also added check in eval\_application to see if the right number of args was provided
- Created test.h to hold all of the color formatting macros to be used in tests

Issues:
1. Evaluating binops can result in a double free
    - When a binop can be "collapsed" into a single value, the original tree is freed
    - The original tree still has a pointer to the binop that has been freed
    - The original tree is freed later on (including the already freed binop)

Fixes:
1. Issue 1 (binop double free)
    1. ~~Pass in a double pointer to eval and eval\_binop, so the tree pointer can be changed directly~~
    2. Make evaluation entirely in-place for the parse tree, freeing and adjusting pointers along the way

---

### June 2nd, 2024

Goals for today:
- [x] Write plenty of tests for eval.c
    - Not completely done
    - I wrote quite a few tests, but I want to implement all of the parser\_tests outputs as tests for eval
- [ ] Begin work on main cli loop (deferred)

Notes:
- Decided to check env as well for eval tests
    - To do this, I started work on a env\_to\_str function
- When implementing env\_to\_str I found that sprintf is generally better than strcat
    - I'm not currently planning to go back and replace all of my calls to strcat, but maybe in the future
    - I know snprintf is safer, but I'm not too concerned about that
- I decided to add enviroments as inputs for eval\_tests in addition to parse trees

Issues:
1. String sizing for env\_to\_str function
    - I'm implementing env\_to\_str just by looping through env linked list and concatting id and data (parse tree str)
    - For the data I can simply call expr\_tree\_to\_str
    - The problem is not knowing the size of data\_str when initially malloc'ing memory for the entire string
2. Environment stores pointers not copies
    - The env can be affected when the parse tree is evaluated in place since pointers in env can become dangling

Fixes:
1. Issue 1 (env\_to\_str)
    - Implement it recursively
2. Issue 2 (env not storing copies)
    - Store copies, but I need to implement a copy\_expr\_tree function first

---

### June 4th, 2024

Goals for today:
- [x] Write a few more tests for eval.c

Notes:
    - Development is going to be a lot slower except for weekends going forward because I'm working as an intern 9-5

Issues:
1. Function evaluation runs into "unbound identifier" when the body is evaluated

---

### June 7th, 2024

Goals for today:

- [x] Fix issue from June 4th
- [ ] Continue writing tests for eval.c (deferred)

Notes:

- From this point forward the fixes will be in an unordered list because I name the specific issue they address anyways

Issues:

1. Invalid free in fun defn eval test
    - Looks like its related to free\_env based on valgrind output

Fixes:

- Issue 1 from June 4th (function body evaluation)
    - Push params onto env before eval (w/temp value of their own ID) and pop params after eval
- Issue 1 from today (invalid free)
    - Fixed copy\_expr\_tree: it was shallow copying function IDs, so I made it a deep copy

---

### June 8th, 2024

Goals for today:

 - [ ] Continue writing tests for eval.c

