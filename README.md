# CS 214 Project III — mysh (PARTIAL IMPLEMENTATION)

Partners: **ADD BOTH NAMES AND NETIDs BEFORE SUBMISSION**
Due: November 11, 2026, 11:59 PM ET

## Current implementation (Partner A)
- `mysh.c`: unified interactive/batch input loop using POSIX `read()`; reads only through the next newline.
- Optional script file argument; terminal detection with `isatty()`; interactive prompt showing `~` for paths under `HOME`, welcome/goodbye.
- Parsing of whitespace-delimited tokens, inline comments beginning with `#`, empty lines.
- Built-ins without operators: `cd`, `pwd`, `which`, `exit`.
- Fixed path search order for `which`: `/usr/local/bin`, `/usr/bin`, `/bin`.
- Basic test runner `test_basic.sh`.

## Remaining work (Partner B)
Replace the stub in `execute.c`, implementing `execute_job` per `mysh.h`:
1. Execute external programs with `fork()`, `execv()`, and `waitpid()`; implement program lookup using the three required directories. Do not use `execvp`.
2. Implement wildcard expansion (`*`) of argument tokens, including path-prefix patterns, hidden-file matching rule, and pass-through of unmatched patterns.
3. Implement `<`, `>`, and `>>` using `open()` and `dup2()`. Set created file permissions to `0640`.
4. Implement arbitrary-length pipelines with `pipe()` and `dup2()`; success/failure depends on last process.
5. Support built-ins `cd`, `pwd`, `which`, `exit` when used in a pipeline or with redirection (potentially duplicate or refactor built-in functions from `mysh.c` for reuse).
6. In batch mode use `/dev/null` as child stdin by default; for interactive mode preserve terminal stdin unless overridden per spec.
7. Report signals and nonzero statuses in interactive mode; coordinate with Partner A because `execute_job` currently returns only integer status, so communicate signaled termination distinctly (e.g., add an out parameter).
8. Validate syntax errors and handle them without exiting the whole shell. If `exit` occurs anywhere in a job, set `*exit_requested = 1` after all pipeline processes finish.
9. Add thorough integration tests and final README test plan.

**Integration note:** `mysh.c` sends any command that contains `|`, `<`, `>`, `>>`, or a non-built-in command to `execute_job`. All tokens remain valid for the duration of the function call. Wildcards also pass through as literal argument tokens until Partner B expands them. Built-ins without operators already work. Do not call `free()` on tokens.

## Build and test
```sh
make
sh test_basic.sh
./mysh
./mysh commands.txt
printf 'pwd\nexit\n' | ./mysh
make clean
```

## Preliminary test plan
- Empty commands, comment-only lines, comments after arguments.
- `pwd`, `cd /`, `cd` (HOME), invalid directory, too many cd arguments.
- `which ls`, `which /bin/ls`, missing command, multiple names, zero names.
- Interactive prompt and exit; input from stdin pipe and from script file.
- EOF without trailing newline.
- **Partner B must add:** external commands with arguments, wildcard combinations, file redirections, multi-stage pipes, all error cases, and built-ins with operators.

## Submission
Create a tar archive with all source files, Makefile, README and tests. Do not submit until `execute.c` stub has been replaced, and both partners' names/NetIDs are listed.
