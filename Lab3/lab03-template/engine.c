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
    // Convert command to absolute path if needed (e.g., "ls" -> "/bin/ls")
    // Returns TRUE if command was found, FALSE otherwise
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
            char* command; // Command path to run
            char* args;    // Arguments to pass to command
            char* envp[] = {NULL};  // Environment variables for command
            if (tokens[2]->type == TOKEN_REDIR){
                int fd = open(tokens[3]->value, O_WRONLY);
                dup2(fd, STDOUT_FILENO);
                close(fd);
                command = tokens[0]->value;
                args = tokens[1]->value;
            }
            execve(command, args, envp);
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


