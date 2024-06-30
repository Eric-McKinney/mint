# Log

## Preamble

I'm starting this log to keep myself focused, track my progress, and to be able to reference it later. \
The format for each entry is as follows:

- A goals section describing what I intend to get done (checked off as I finish each thing)
- A notes section for any significant unplanned changes I make or just to say everything went to plan
- An optional issues section where I describe problems I'm having
- An optional fixes section where I describe the solutions/fixes to the problems in the issues section

I wish I thought of this earlier, but it is what it is.

## June 1st, 2024

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

## June 2nd, 2024

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

## June 4th, 2024

Goals for today:

- [x] Write a few more tests for eval.c

Notes:

- Development is going to be a lot slower except for weekends going forward because I'm working as an intern 9-5

Issues:

1. Function evaluation runs into "unbound identifier" when the body is evaluated

## June 7th, 2024

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

## June 8th, 2024

Goals for today:

- [x] Continue writing tests for eval.c

Notes:

- I added CI via GitHub actions, but that brings up issue 1 of today
- I also added a badge for the CI on the README

Issues:

1. Tests exit code and failure behavior
    - No matter what each test run ends with an exit code of 0 assuming it runs to completion
    - If it doesn't run to completion because a test failed and exited, the rest of that test suite is not run

## June 9th, 2024

Goals for today:

- [x] Restructure test suites (issue 1 from yesterday)

Notes:

- I'm heavily considering not restructuring the parser and lexer tests since I don't think I'll add more tests later,
  but I should probably do at least the parser tests for consistency's sake and the potential that I will add tests

Issues:

1. Fork copies the heap, so now each test creates a bunch of unused memory and leaks it
2. Strange output duplication in eval tests only when output is redirected
    - The first print in each child process seems to print a copy of all that came before it

Fixes:

- Issue 1 from yesterday (test behavior)
    - Restructure, forking for each test to be run in a child process
- Issue 1 from today (test leaking memory)
    - Create inputs within each test & generally make each test self-contained
- Issue 2 from today (output duplication on output redirect)
    - Turns out that forking copies the IO buffer
    - On normal stdout with no redirect this was not a problem because the buffer is flushed on newline
    - When redirected the buffer is **not** flushed on newline

## June 15th, 2024

Goals for today:

- [x] Continue and finish writing eval tests (if possible)
- [ ] Use a pipe to pass env changes from child to parent (deferred)

Notes:

- Regarding the note from 6/9/24, I think I'll just leave the lexer tests format alone for good and I'll leave the
  parser tests format alone until I add more tests

Issues:

1. The heap is not shared between the parent and child process, so the env cannot be updated from child to parent after
   an assignment or function has been evaluated (or at least not easily)

Fixes:

- Issue 1 from today (forking makes separate heaps -> env doesn't update)
    - I thought I could use threads since they share a heap, but if one thread exits or segfaults then the main thread
  ends which defeats the purpose
    - There appear to be a few ways for processes to communicate, but I think a pipe is the most sane way to do it

## June 16th, 2024

Goals for today:

- [x] Write a function to serialize an env (either the whole thing or just the changes to be made to the env)
- [ ] Write a function to deserialize an env (deferred)
- [ ] Write a function to print each eval result as an end user would expect to see them (deferred)
- [ ] Use a pipe to allow serialized envs to be sent/received between parent and child processes (deferred)

Notes:

- So as it turns out, this whole setup (fork, exit on error, serializing env & sending it through a pipe) is a real pain
    - For now, I'll make this whole serializing business its own branch
    - I'll make another branch different error handling using errno instead of exiting, then maybe this can all be done
  in one process

## June 22nd, 2024

Goals for today:

- [x] Learn about errno
- [x] See if I can adapt the current error handling to use errno instead of exiting
- [x] Update error handling to errno for lexer

Notes:

- After investigating errno and the many error reporting functions, I see a definite way forward with errno which will
  make my life a lot easier, no pipes necessary

## June 23, 2024

Goals for today:

- [x] Update error handling to errno for parser
- [ ] Update error handling to errno for evaluator (deferred)

Notes:

- Changing error handling in parser to errno was much more of a pain than I anticipated
    - I need to revisit this and make error messages more helpful and limit malformed inputs to 1 error message
- I also checked and there are no memory leaks for the bad inputs I gave

## June 29th, 2024

Goals for today:

- [x] Update error handling to errno for evaluator

Issues:

1. I have found a few lingering problems from moving to errno in eval in the following commands
    - `y = y` (y not previously defined)
    - `fn a(x, y) = x + y` followed by `a(1,2,3)` (too many args)

## June 30th, 2024

Goals for today:

- [x] Fix error handling for eval (issue 1 from yesterday)

Issues:

1. I fixed issue 1 from yesterday mostly, as no crashes occur, but the too many args case doesn't throw any errors

