#include "mysh.h"
#include <stdio.h>

/* TEAMMATE TODO: replace this stub with the full job executor.
 * argv[0..count-1] includes the program and operators as tokens.
 * See mysh.h and README.md for requirements.
 */
int execute_job(char *argv[], int count, int interactive, int *exit_requested) {
    (void)argv;
    (void)count;
    (void)interactive;
    (void)exit_requested;
    fprintf(stderr, "mysh: external commands, pipes and redirection not implemented yet\n");
    return 1;
}
