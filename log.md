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
- [ ] Write plenty of tests for eval.c
- [ ] Begin work on main cli loop

Notes:
