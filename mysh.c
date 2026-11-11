#define _POSIX_C_SOURCE 200809L
#include "mysh.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static int read_line(int fd, char **line, size_t *capacity) {
    size_t length = 0;
    for (;;) {
        char ch;
        ssize_t n = read(fd, &ch, 1);
        if (n == 0) {
            if (length == 0) return 0;
            break;  /* execute final line even if it lacks newline */
        }
        if (n == -1) {
            if (errno == EINTR) continue;
            perror("mysh: read");
            return -1;
        }
        if (length + 1 >= *capacity) {
            size_t next = *capacity ? (*capacity * 2) : 128;
            char *temp = realloc(*line, next);
            if (!temp) {
                perror("mysh: realloc");
                return -1;
            }
            *line = temp;
            *capacity = next;
        }
        if (ch == '\n') break;
        (*line)[length++] = ch;
    }
    (*line)[length] = '\0';
    return 1;
}

/* Tokenize in place. Assignment permits special operators to be surrounded
 * by whitespace. An inline # starts a comment, so ignore the rest.
 * Returns number of tokens; caller frees the token pointer array, not tokens.
 */
static int tokenize(char *line, char ***result) {
    char *comment = strchr(line, '#');
    if (comment) *comment = '\0';
    int count = 0;
    size_t cap = 8;
    char **tokens = malloc(cap * sizeof(*tokens));
    if (!tokens) return -1;
    char *save = NULL;
    for (char *word = strtok_r(line, " \t\r\v\f", &save);
         word; word = strtok_r(NULL, " \t\r\v\f", &save)) {
        if ((size_t)count + 1 >= cap) {
            cap *= 2;
            char **tmp = realloc(tokens, cap * sizeof(*tokens));
            if (!tmp) { free(tokens); return -1; }
            tokens = tmp;
        }
        tokens[count++] = word;
    }
    tokens[count] = NULL;
    *result = tokens;
    return count;
}

/* Return a newly allocated executable path, or NULL if not executable. */
static char *find_executable(const char *name) {
    if (strchr(name, '/')) {
        return access(name, X_OK) == 0 ? strdup(name) : NULL;
    }
    const char *dirs[] = {"/usr/local/bin", "/usr/bin", "/bin"};
    for (size_t i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++) {
        size_t needed = strlen(dirs[i]) + 1 + strlen(name) + 1;
        char *path = malloc(needed);
        if (!path) return NULL;
        snprintf(path, needed, "%s/%s", dirs[i], name);
        if (access(path, X_OK) == 0) return path;
        free(path);
    }
    return NULL;
}

static int is_builtin(const char *name) {
    return strcmp(name, "cd") == 0 || strcmp(name, "pwd") == 0 ||
           strcmp(name, "which") == 0 || strcmp(name, "exit") == 0;
}

/* Only plain built-ins are handled here. Built-ins with |, <, >, >>
 * are delegated to execute_job so the teammate can apply dup2/pipe.
 */
static int run_builtin(char *argv[], int count, int *exit_requested) {
    if (strcmp(argv[0], "exit") == 0) {
        *exit_requested = 1;
        return 0;
    }
    if (strcmp(argv[0], "cd") == 0) {
        if (count > 2) {
            fprintf(stderr, "cd: too many arguments\n");
            return 1;
        }
        const char *destination = count == 2 ? argv[1] : getenv("HOME");
        if (!destination || chdir(destination) == -1) {
            perror("cd");
            return 1;
        }
        return 0;
    }
    if (strcmp(argv[0], "pwd") == 0) {
        if (count != 1) {
            fprintf(stderr, "pwd: expected no arguments\n");
            return 1;
        }
        char *cwd = getcwd(NULL, 0);
        if (!cwd) { perror("pwd"); return 1; }
        puts(cwd);
        free(cwd);
        return 0;
    }
    if (strcmp(argv[0], "which") == 0) {
        if (count == 1) {
            fprintf(stderr, "which: missing program name\n");
            return 1;
        }
        int ok = 1;
        for (int i = 1; i < count; i++) {
            char *path = find_executable(argv[i]);
            if (path) { puts(path); free(path); }
            else { fprintf(stderr, "which: %s: not found\n", argv[i]); ok = 0; }
        }
        return ok ? 0 : 1;
    }
    return 1;
}

static void print_prompt(void) {
    char *cwd = getcwd(NULL, 0);
    const char *home = getenv("HOME");
    if (!cwd) { perror("mysh: getcwd"); fputs("$ ", stdout); }
    else if (home && *home && strncmp(cwd, home, strlen(home)) == 0 &&
             (cwd[strlen(home)] == '/' || cwd[strlen(home)] == '\0')) {
        printf("~%s$ ", cwd + strlen(home));
    } else printf("%s$ ", cwd);
    free(cwd);
    fflush(stdout);
}

int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [script-file]\n", argv[0]);
        return EXIT_FAILURE;
    }
    int fd = STDIN_FILENO;
    if (argc == 2) {
        /* open required for script argument; unlike stdin, we own this fd */
        fd = open(argv[1], O_RDONLY);
        if (fd == -1) { perror(argv[1]); return EXIT_FAILURE; }
    }
    int interactive = (argc == 1 && isatty(STDIN_FILENO));
    if (interactive) puts("Welcome to my shell!");
    char *line = NULL;
    size_t capacity = 0;
    int exit_requested = 0;
    while (!exit_requested) {
        if (interactive) print_prompt();
        int r = read_line(fd, &line, &capacity);
        if (r == 0) break;
        if (r < 0) break;
        char **tokens = NULL;
        int count = tokenize(line, &tokens);
        if (count < 0) { perror("mysh: tokenize"); break; }
        if (count == 0) { free(tokens); continue; }
        int special = 0;
        for (int i = 0; i < count; i++) {
            if (!strcmp(tokens[i], "|") || !strcmp(tokens[i], "<") ||
                !strcmp(tokens[i], ">") || !strcmp(tokens[i], ">>")) {
                special = 1;
                break;
            }
        }
        int status;
        if (!special && is_builtin(tokens[0]))
            status = run_builtin(tokens, count, &exit_requested);
        else
            status = execute_job(tokens, count, interactive, &exit_requested);
        if (interactive && status != 0 && !exit_requested)
            fprintf(stderr, "Exited with status %d\n", status);
        free(tokens);
    }
    free(line);
    if (fd != STDIN_FILENO) close(fd);
    if (interactive) puts("Exiting my shell.");
    return EXIT_SUCCESS;
}
