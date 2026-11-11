#ifndef MYSH_H
#define MYSH_H

/* Shared contract between both partners.
 * argv contains count whitespace-delimited tokens, without comments.
 * argv[count] == NULL. Tokens are owned by main; do not free them.
 * This function must support external programs, redirection, pipelines,
 * wildcard expansion, and built-ins inside pipelines or redirections.
 * Return 0 for success, nonzero for failure. Set *exit_requested = 1
 * when an 'exit' command appears anywhere in the job.
 */
int execute_job(char *argv[], int count, int interactive, int *exit_requested);

#endif
