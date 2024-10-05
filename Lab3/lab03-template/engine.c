#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include "parser.h"
#include <string.h>

int read_line(int infile, char *buffer, int maxlen)
{
    int readlen = 0;
    char ch;
    ssize_t result;

    while (readlen < maxlen - 1) {
        result = read(infile, &ch, 1);
        if (result == 0) { // EOF
            break;
        } else if (result < 0) { // Error
            perror("read");
            return -1;
        }

        buffer[readlen++] = ch;
        if (ch == '\n') {
            break;
        }
    }

    buffer[readlen] = '\0'; // Null-terminate the string
    // TODO: Read a single line from file; retains final '\n'
    
    return readlen;
}


int normalize_executable(char **command) {
    if (command == NULL || *command == NULL) {
        return FALSE;
    }

    if ((*command)[0] == '/') {
        return TRUE;
    }

    if (strchr(*command, '/') != NULL) {
        return TRUE;
    }

    char *path = getenv("PATH");
    if (path == NULL) {
        return FALSE;
    }

    char *path_copy = strdup(path); 
    char *token = strtok(path_copy, ":");
    while (token != NULL) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", token, *command);

        if (access(fullpath, X_OK) == 0) {
            *command = strdup(fullpath);
            free(path_copy);
            return TRUE;
        }

        token = strtok(NULL, ":");
    }

    free(path_copy);
    return FALSE;
}


void update_variable(char* name, char* value) {
    // Update or create a variable
}

char* lookup_variable(char* name) {
    // Lookup a variable
    return NULL;
}


int main(int argc, char *argv[])
{
    if(argc != 2) {
        printf("Usage: %s <input file>\n", argv[0]);
        return -1;
    }

    int infile = open(argv[1], O_RDONLY);
    if(infile < 0) {
        perror("Error opening input file");
        return -2;
    }

    char buffer[1024];
    int readlen;
    
    // Read file line by line
    while( 1 ) {

        // Load the next line
        readlen = read_line(infile, buffer, 1024);
        if(readlen < 0) {
            perror("Error reading input file");
            return -3;
        }
        if(readlen == 0) {
            break;
        }

        // Tokenize the line
        int numtokens = 0;
        token_t** tokens = tokenize(buffer, readlen, &numtokens);
        assert(numtokens > 0);

        // Parse token list
        // * Organize tokens into command parameters
        // * Check if command is a variable assignment
        // * Check if command has a redirection
        // * Expand variables if any
        // * Normalize executables
        // * Check if pipes are present

        // * Check if pipes are present
        // TODO

        char* command = tokens[0]->value;
        char* args[numtokens + 1];
        for (int i = 0; i < numtokens; i++) {
            args[i] = tokens[i]->value;
        }
        args[numtokens] = NULL;

        // Normalize the executable path
        if (!normalize_executable(&command)) {
            fprintf(stderr, "Command not found: %s\n", command);
            continue;
        }
        // Run commands
        pid_t pid = fork();
        // * Fork and execute commands
        // * Handle PATH
        // * Handle redirections
        // * Handle pipes
        // * Handle variable assignments
        // TODO
        // Fork and execute commands base template
        if (pid == 0) {
            // Child process
            execve(command, args, NULL);
            perror("Error executing command");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            // Parent process
            int status;
            waitpid(pid, &status, 0);
        } else {
            perror("Error forking");
            return -4;
        }
        // Free tokens vector
        for (int ii = 0; ii < numtokens; ii++) {
            free(tokens[ii]->value);
            free(tokens[ii]);
        }
        free(tokens);
    }

    close(infile);
    
    // Remember to deallocate anything left which was allocated dynamically
    // (i.e., using malloc, realloc, strdup, etc.)

    return 0;
}


