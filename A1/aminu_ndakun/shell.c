#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 1024

int main(int argc, char **argv) {
    char line[MAX_LINE_LENGTH];
    char *args[MAX_LINE_LENGTH];
    int status;

    while (1) {
        // Print prompt
        char *cwd = getcwd(NULL, 0);
        printf("%s$ ", cwd);
        free(cwd);

        // Read line
        if (fgets(line, MAX_LINE_LENGTH, stdin) == NULL) {
            break;
        }

        // Split line into arguments
        char *getcmd = strtok(line, " \t\n");
        int i = 0;
        while (getcmd != NULL) {
            args[i] = getcmd;
            getcmd = strtok(NULL, " \t\n");
            i++;
        }
        args[i] = NULL;

        // Check for built-in commands
        if (args[0] == NULL) {
            // Do nothing if no command is given
            continue;
        } else if (strcmp(args[0], "exit") == 0) {
            // Exit the shell
            if (args[1] != NULL) {
                fprintf(stderr, "Usage Error: exit takes no arguments\n");
            } else {
                exit(0);
            }
        } else if (strcmp(args[0], "cd") == 0) {
            // Change directory
            if (args[1] == NULL) {
                fprintf(stderr, "Usage Error: cd requires one argument\n");
            } else {
                if (chdir(args[1]) != 0) {
                    fprintf(stderr, "Error: %s\n", strerror(errno));
                }
            }
        } else if (strcmp(args[0], "exec") == 0) {
            // Execute command
            if (args[1] == NULL) {
                fprintf(stderr, "Usage Error: exec requires at least one argument\n");
            } else {
                execv(args[1], args + 1);
                fprintf(stderr, "Error: %s\n", strerror(errno));
            }
        } else {
            // Check if command is a path
            if (args[0][0] == '.' || args[0][0] == '/') {
                // Create child process
                pid_t pid = fork();
                if (pid == 0) {
                    // Execute command in child process
                    execv(args[0], args);
                    fprintf(stderr, "Error: %s\n", strerror(errno));
                    exit(1);
                } else {
                    // Wait for child process to finish
                    waitpid(pid, &status, 0);
                }
            } else {
                // Print unrecognized command
                fprintf(stderr, "Unrecognized command: %s\n", args[0]);
            }
        }
    }

    return 0;
}
