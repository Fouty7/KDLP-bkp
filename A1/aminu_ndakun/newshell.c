#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_LINE_LENGTH 1024
#define MAX_ARGS 64

// function prototypes
void parse_input(char *input, char **args);
int execute_command(char **args);
int builtin_command(char **args);
int exec_file(char **args);

int main(int argc, char **argv) {
    char line[MAX_LINE_LENGTH];
    char *args[MAX_ARGS];

    while (1) {
        // print prompt
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("%s$ ", cwd);

        // read user input
        if (fgets(line, MAX_LINE_LENGTH, stdin) == NULL) {
            break;
        }

        // parse input
        parse_input(line, args);

        // execute command
        if (args[0] != NULL) {
            if (execute_command(args) == -1) {
                break;
            }
        }
    }

    return 0;
}

void parse_input(char *input, char **args) {
    // split input into pieces delimited by whitespace
    int i = 0;
    args[i] = strtok(input, " \t\r\n\a");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " \t\r\n\a");
    }
}

int execute_command(char **args) {
    // check for builtin commands
    if (builtin_command(args) == 0) {
        return 0;
    }

    // check for executable files
    if (exec_file(args) == 0) {
        return 0;
    }

    // unrecognized command
    printf("Unrecognized command: %s\n", args[0]);
    return 0;
}

int builtin_command(char **args) {
    // exit
    if (strcmp(args[0], "exit") == 0) {
        if (args[1] == NULL) {
            exit(0);
        } else {
            printf("Error: exit takes no arguments\n");
        }
        return 0;
    }

    // cd
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            printf("Error: cd takes one argument\n");
        } else {
            if (chdir(args[1]) != 0) {
                printf("Error: %s\n", strerror(errno));
            }
        }
        return 0;
    }

    // exec
    if (strcmp(args[0], "exec") == 0) {
        if (args[1] == NULL) {
            printf("Error: exec takes at least one argument\n");
        } else {
            if (execv(args[1], args + 1) == -1) {
                printf("Error: %s\n", strerror(errno));
            }
        }
        return 0;
    }

    // not a builtin command
    return -1;
}

int exec_file(char **args) {
    char *path;
    char *file;
    int i;

    // check if first arg is a path
    if (args[0][0] == '.' || args[0][0] == '/') {
        // create child process
        pid_t pid = fork();
        if (pid == 0) {
            // child process
            if (execv(args[0], args) == -1) {
                printf("Error: %s\n", strerror(errno));
                exit(1);
            }
        } else if (pid > 0) {
            // parent process
            waitpid(pid, NULL, 0);
            return 0;
        } else {
            // fork failed
            printf("Error: %s\n", strerror(errno));
            return -1;
        }
    }

    // search PATH for file
    path = getenv("PATH");
    file = strtok(path, ":");
    while (file != NULL) {
        // build path
        char full_path[MAX_LINE_LENGTH];
        strcpy(full_path, file);
        strcat(full_path, "/");
        strcat(full_path, args[0]);

        // check if file exists
        struct stat st;
        if (stat(full_path, &st) == 0) {
            // create child process
            pid_t pid = fork();
            if (pid == 0) {
                // child process
                if (execv(full_path, args) == -1) {
                    printf("Error: %s\n", strerror(errno));
                    exit(1);
                }
            } else if (pid > 0) {
                // parent process
                waitpid(pid, NULL, 0);
                return 0;
            } else {
                // fork failed
                printf("Error: %s\n", strerror(errno));
                return -1;
            }
        }

        // get next file
        file = strtok(NULL, ":");
    }

    // file not found
    return -1;
}
