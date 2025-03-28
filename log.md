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
- [ ] Write a function to print each eval result as an end user would expect to see them (deferred)

Notes:

- I decided to put guards which check errno for each function that adds to the env, so the env can't be polluted in an
  error case

Issues:

1. I fixed issue 1 from yesterday mostly, as no crashes occur, but the too many args case doesn't throw any errors
2. I found more buggy cases
    - `fn f() = 3` (function defn with no params)
    - `f()` (function with no args)
    - I realize this is a parsing issue because my CFG doesn't allow for these cases, ~~but I think it should~~
    - Followup to the above: I don't think it makes sense to allow functions with no parameters, but it definitely
  shouldn't crash the program. It should also probably have a proper error message
3. In parse and eval I should add more null checks since any eval/parse could potentially return null if it runs into
  an error
    - ex: `f()` case generates a segfault because parsing dereferences a null pointer when it checks for a comma in args

Fixes:

- Issue 1 from yesterday (both of the problem cases)
    - There was a double free in main.c because I forgot to put a return when an error is found after eval
- Issue 1 from today (too many args case)
    - The check for num\_params != num\_args\_bound doesn't catch too many args because bind\_args stops at the end of
  the parameters
    - The fix is to count the number of args (not just the number of args bound to parameters)

## July 4th, 2024

Goals for today:

- [x] Write a function to print each eval result as an end user would expect to see them

Notes:

- Happy independence day 🎉
- I think from now on the role of my issues section in this log is going to change now that I'm using GitHub issues
    - I will use GitHub issues for bugs I find and quick todo items
    - I will use the issues here for hurdles I run into while trying to implement/fix something

## August 3rd, 2024

Goals for today:

- [x] Write the "Reporting bugs" wiki page
- [ ] Write the "Using mint" wiki page (deferred)

Notes:

- It's been a while
- I have been working on the wiki lately which is all through GitHub, so I forgot to update this log
- I also streamlined a few other things in the GitHub like issue templates
- I've pretty much completely switched to using GitHub issues for my backlog (another reason I forgot to update this log)
- I randomly decided to stop the prompt from printing when input redirection is used (huge QoL improvement)

## September 14th, 2024

Goals for today:

- [x] Finish writing the "Using mint" wiki page
- [x] Clean up README

Notes:

- I've been slacking on this project big time
- I have time to work on mint, but I just haven't been doing that
    - To be fair I've been working on some other projects 
- The semester started, but I should still have plenty of free time

## October 5th, 2024

Goals for today:

- [x] Implement exponents
- [x] Implement comments
- [x] Take steps toward readline support
- [x] Update wiki when done w/changes

Notes:

- Enough slacking, I need to get working
- Just finished exponents. We are so back
    - Should have used a new branch, but whatever lol it worked out
- Some thinking: how to handle comments
    - I could have comments be limited to being on their own line which I think looks cleaner
    - Although I'm not sure it makes sense to restrict user choice bc of my own stylistic preferences
    - Alright it's decided, comments can happen on their own line or the end of any other line
- Finished comments too! I forgot how much fun this is
- Made a branch for readline support and did a lot of thinking
    - readline doesn't capture the newline
    - I probably don't need to pay attention to newlines anyways (so remove TOK\_ENDLN & remove \n from CFG)
    - For invoking mint on a file, use fgets and discard newline (also change print behavior)
    - Does it even make sense to call mint on a file at all? (instead have import functionality in shell)
    - I also want to enable use of mint in shell scripts (i.e. `mint 1 + 2` would yield 3 in a script)

## October 6th, 2024

Goals for today:

- [x] Progress on switching to readline

Notes:

- Ok so here's the decision on invocation behavior
    - With no additional arguments, mint will start the shell/REPL
    - Otherwise all additional arguments will be treated as an expression and the result of evaluation will be printed
- In the future I want some sort of import statement which can run files through the interpreter, adding to the env
    - Maybe have a flag like `-f` or something to run mint on a file from the command line
- I also want to keep support for input redirection

## October 7th, 2024

Goals for today:

- [x] Get working repl and input redirection with readline and fgets respectively

Notes:

- Midterms coming up, so I will probably have to take a break after this

Issues:

1. Weird bug where any regex with a bracket in it would have regcompile set errno to 84, but still returned 0?
    - errno 84 = EILSEQ = Invalid or incomplete multibyte or wide character
    - Also only in repl where the line was scanned in with readline
    - The only problem with this is that the regcompile call is with THE SAME STRING LITERAL!!!
    - Clearly readline is touching some shared global variable similar to errno (but not errno) which messes things up
2. Had issues with stack smashing after removing TOK\_ENDLN and mentions of \n in general
    - As it turns out, when I changed re\_match back to being just one struct, I forgot to update the args on one call
    - That call happened to be the comment\_re case where it was trying to jam two match structs into the space of one
    - Thus, the stack had been smashed (oopsies xd)

Fixes:

1. Move the call to regcomp
    - I was meaning to do this anyways because it doesn't make sense to compile the regexs for every tokenize call
    - Now regexs are compiled when the env is initialized and they are freed at the very end
    - Now I just need to update my tests
2. After many minutes of debugging, I identified the argument I forgot to update and updated 1 character to fix it LOL
    - Literally changed a 2 to a 1 and all is good now

## October 8th, 2024

Goals for today:

- [x] Do some final manual testing of readline-support branch for different invocation methods
- [x] Merge and close out readline-support branch
- [x] Update wiki

## October 11th, 2024

Goals for today:

- [ ] Add error for integer overflow (deferred)
- [ ] Allow longer variable names or add error for exceeding max length (deferred)
- [ ] Fix accidental closures (evaluation of function and variable definitions) (deferred)

Notes:

- Forgot to make an entry yesterday, but here's what I did:
    - Finally switched to long int, merging Justin's PR in the process
    - Fixed some small bugs related to parsing undefined function applications and division by 0

## October 13th, 2024

- [ ] See if it makes sense to store all numbers as doubles (deferred)
- [x] Add error for integer overflow
- [ ] Allow longer variable names or add error for exceeding max length (deferred)
- [ ] Fix accidental closures (evaluation of function and variable definitions) (deferred)

Notes:

- So it turns out I was busier than expected outside of this project yesterday and the day before
- It also turns out that checking for integer over/underflow was a little more difficult than anticipated
    - I also began to think about float over/underflow
    - Now I'm thinking it might just make more sense to store all numbers as doubles
- As it turns out, I'm still busy lol (and will be until midterms are over)
- I did add errors for integer overlow in a new branch, but I'm holding off on merging it
    - First I want to see how viable using only doubles for everything is
    - If that doesn't go well, I'll tidy up the existing code

## October 19th, 2024

Goals for today:

- [x] See if it makes sense to store all numbers as doubles
- [x] Tidy up existing eval code
- [ ] Allow longer variable names or add error for exceeding max length (deferred)
- [ ] Fix accidental closures (evaluation of function and variable definitions) (deferred)

Notes:

- Midterms are over
- I don't want to go the route of doubles only because it feels hacky and potentially causes a loss of precision
- Much happier with eval code after tidying up eval\_binop (although it is still very long)
- Despite my newfound free time I am still loafing around
- Maybe I will make an effort to work on mint this week (big maybe though)

## October 24th, 2024

Goals for today:

- [x] Fix accidental closures
- [x] Add check for error correctness in eval tests
- [x] If there's time replace "MAX\_LENGTH" type macros with a way to allow indefinite length values to be printed
    - (i.e. IDs and numbers)
    - There was not time lol

Notes:

- As it would turn out, it was a big maybe that I would make an effort this week lol
- Hopefully I can get some solid work in today, but I don't expect too much
- Same goes for tomorrow and this weekend, but realistically I'm busy doing stuff with friends
- Fixed accidental closures, but want to add tests for them before merging

## October 26th, 2024

Goals for today:

- [x] Rework eval test infrastructure
- [x] Merge accidental closure fix

Notes:

- The eval tests are kinda sad and poorly structured, so I redid them
    - The old format was mostly to allow for custom-built input trees to test only eval stuff
    - In reality it was clunky and took up a lot of space
    - Sending everything through the lexer and parser is just simpler and harder to screw up on the testing side
    - May lead to finding more bugs to fix too
    - Also there's not much merit in testing just eval because it will never be used on its own in practice
    - Also also the lexer and parser tests should weed out most of the bugs related to their parts, so it not as bad
- Pretty hard to test for the closure fix since it involves evaluating several expressions in the same env
    - I did test manually of course, but it would be better to have automated tests
    - For now I'll let it slide

## October 27th, 2024

Goals for today:

- [x] Add errors for functions with no params and params w/same name as function
- [ ] Add tests for the above fixes (halfway done)

Notes:

- Gonna need to redo the parser tests like I did for eval so I can test for errors (tomorrow or whenever I get to it)

## November 2nd, 2024

Goals for today:

- [x] Rewrite parser test infrastructure
- [x] Add tests for error when functions or function applications have no params

Notes:

- Felt good to clean up parser tests like that
- I should probably get around to doing the same with the lexer tests
- I also need to add more error test cases for the parser
    - There are sooooooo many possible malformed inputs
    - I also want to check that there are no memory leaks

## November 8th, 2024

Goals for today:

- [x] Rewrite lexer test infrastructure
- [x] Add some error test cases for lexer
- [ ] Add more error test cases for parser (deferred)
    - Ending input early
    - Missing operands
    - Missing operators

## November 11th, 2024

Goals for today:

- [x] Add more error tests (see previous entry)
- [ ] Add functions to lexer.c for converting tokens to strings with just their values (deferred)

Notes:

- Could make do with even more parser tests tbh (there's a lot of possible malformed inputs)

## November 12th, 2024

Goals for today:

- [x] Add functions to lexer.c for converting tokens to strings with just their values
- [x] Switch some error messages to use these ^ new functions
- [x] Add error codes/numbers to errors for easier reference in and by the wiki

Notes:

- Devising a system for error codes is more complicated than I would have initially imagined
    - It should be easy to add new codes without having to do anything to existing codes
    - Codes should all be the same length
    - Codes should be unique for each error message
    - Ideally the code would convey some information about the error
    - Realistically it's better to simply devise a system to quickly and easily create unique codes
    - Something like E1234
    - I'd like to at least convey which of lexer, parser, or evaluator the error occurred in
    - Would be nice to convey if the error was due to bad input or internal mishaps
- Here's what I'm settling on for error code formatting
    - Starts with E and then 4 digits (so hypothetically 9999 possible codes which should be way, way more than enough)
    - Realistically not using E0000 so we can split 3 ways evenly between lexer, parser, and evaluator
    - However, I doubt that errors will occur evenly between the three (so lets say 10% lexer, 60% parser, & 30% eval)
    - I'll allocate 0000-0999 to lexer, 1000-6999 to parser, and 7000-9999 to evaluator
    - Errors solely due to internal mishaps will be divisible by 5 and the rest are for everything else
    - When I say "solely due to internal mishaps" I mean something that shouldn't be caused by user input
    - (e.g. the default case of a switch statement which shouldn't be possible to reach)
    - When I say "everything else" I mean if it's not 100% certain to be a so called "internal mishap" then it counts
- Perhaps I should consider adding some sort of logging at some point
    - Well that's a problem for later tbh
    - Could be useful for diagnosing failed tests though

## November 13th, 2024

Goals for today:

- [ ] Document every currently existing error for mint in the wiki (deferred)

## November 16th, 2024

Goals for today:

- [ ] Continue documenting every currently existing error for mint in the wiki (deferred)

## December 17th, 2024

Goals for today:

- [ ] Finish(?) documenting errors in the wiki (deferred)

Notes:

- Aaaaaannd it's been a month 😭
- Well the semester is over now, so I will hopefully put more time into this project
- Yeah too many more errors to document to finish today
- Also this is so terribly boring and I feel like I'm not writing it well

## January 19th, 2025

Notes:

- Yeah I decided working on this project while doing a winter class was a bit much, so I uh... didn't
- Probably won't do anything noteworthy until next semester starts bc I want to enjoy the rest of my break

## March 26th, 2025

Notes:

- I may have lied about starting up when the semester started
- Back bc I found an annoying bug when doing my hw using mint
- Not gonna make any more promises about continuing development since I clearly don't honor them
- That being said, I don't plan to drop this project ever (I will simply be inactive for long periods of time)

Issues:

1. Negative exponents dont evaluate properly most of the time
    - I simply didn't account for this, so an integer base and an integer exponent is treated as an integer result

Fixes:

- Issue 1 of today was fixed simply by making a caveat for negative exponents where I don't cast the result to an int
