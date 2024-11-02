#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>

#define TRUE 1
#define FALSE 0
#define MAX_PROCESSES 4

// Output file
FILE* output_file;

// TLB replacement strategy (FIFO or LRU)
char* strategy;

typedef struct {
    uint32_t VPN;  // Virtual Page Number
    uint32_t PFN;  // Physical Frame Number
    int valid;     // Valid bit
} TLB_entry;

typedef struct {
    uint32_t VPN;  // Virtual Page Number
    uint32_t PFN;  // Physical Frame Number
    int valid;     // Valid bit
} PageTable_entry;

typedef struct {
    PageTable_entry* pageTable;
    TLB_entry* TLB;
    uint32_t* physicalMemory;
    int defineCall; 
} Process;

Process processes[MAX_PROCESSES];
int current_process = 0;

int TLB_size = 8;    // Size of the TLB

char** tokenize_input(char* input) {
    char** tokens = NULL;
    char* token = strtok(input, " ");
    int num_tokens = 0;

    while (token != NULL) {
        num_tokens++;
        tokens = realloc(tokens, num_tokens * sizeof(char*));
        tokens[num_tokens - 1] = malloc(strlen(token) + 1);
        strcpy(tokens[num_tokens - 1], token);
        token = strtok(NULL, " ");
    }

    num_tokens++;
    tokens = realloc(tokens, num_tokens * sizeof(char*));
    tokens[num_tokens - 1] = NULL;

    return tokens;
}

void init_process(int pid, int VPN) {
    int pageTable_size = 1 << VPN;
    processes[pid].pageTable = (PageTable_entry*)malloc(pageTable_size * sizeof(PageTable_entry));
    for (int i = 0; i < pageTable_size; i++) {
        processes[pid].pageTable[i].VPN = i;
        processes[pid].pageTable[i].PFN = -1;  // Invalid PFN
        processes[pid].pageTable[i].valid = FALSE;
    }

    processes[pid].TLB = (TLB_entry*)malloc(TLB_size * sizeof(TLB_entry));
    for (int i = 0; i < TLB_size; i++) {
        processes[pid].TLB[i].VPN = -1;  // Invalid VPN
        processes[pid].TLB[i].PFN = -1;  // Invalid PFN
        processes[pid].TLB[i].valid = FALSE;
    }
}

void handle_Define(int OFF, int PFN, int VPN) {
    int length = 1 << (OFF + PFN);
    processes[current_process].physicalMemory = (uint32_t*)calloc(length, sizeof(uint32_t));
    init_process(current_process, VPN);
}

void handle_ctxswitch(int pid) {
    if (pid < 0 || pid >= MAX_PROCESSES) {
        fprintf(output_file, "Current PID: %d. Invalid context switch to process %d\n", current_process, pid);
        return;
    }
    current_process = pid;
    fprintf(output_file, "Current PID: %d. Switched execution context to process: %d\n", current_process, pid);
}

int main(int argc, char* argv[]) {
    const char usage[] = "Usage: memsym.out <strategy> <input trace> <output trace>\n";
    char* input_trace;
    char* output_trace;
    char buffer[1024];

    // Parse command line arguments
    if (argc != 4) {
        printf("%s", usage);
        return 1;
    }
    strategy = argv[1];
    input_trace = argv[2];
    output_trace = argv[3];

    // Open input and output files
    FILE* input_file = fopen(input_trace, "r");
    output_file = fopen(output_trace, "w");  
    

    
    
    while ( !feof(input_file) ) {
        // Read input file line by line
        
        char *rez = fgets(buffer, sizeof(buffer), input_file);
        if ( !rez ) {
            fprintf(stderr, "Reached end of trace. Exiting...\n");
            return -1;
        } else {
            // Remove endline character
            buffer[strlen(buffer) - 1] = '\0';
        }
        char** tokens = tokenize_input(buffer);

        // TODO: Implement your memory simulator

        if (strcmp(tokens[0], "%") == 0) {
           continue;
        }else if(strcmp(tokens[0], "define") == 0) {
            if(processes[current_process].defineCall == 1){
                fprintf(output_file,"Current PID: %d. Error: multiple calls to define in the same trace\n", current_process);
                return -1;
            }
            int OFF = atoi(tokens[1]);
            int PFN = atoi(tokens[2]);
            int VPN = atoi(tokens[3]);
            handle_Define(OFF, PFN, VPN);
            processes[current_process].defineCall = 1;
            fprintf(output_file, "Current PID: %d. Memory instantiation complete. OFF bits: %d. PFN bits: %d. VPN bits: %d\n", current_process,OFF, PFN, VPN);
        }
        else if (strcmp(tokens[0], "ctxswitch") == 0) {
            int pid = atoi(tokens[1]);
            handle_ctxswitch(pid);
        }
        else if (processes[current_process].defineCall == 0) {
            fprintf(output_file,"Current PID: %d. Error: attempt to execute instruction before define\n", current_process);
            return -1;
        }
        // Deallocate tokens
        for (int i = 0; tokens[i] != NULL; i++)
            free(tokens[i]);
        free(tokens);
    }
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].TLB) {
            free(processes[i].TLB);
        }
        if (processes[i].physicalMemory) {
            free(processes[i].physicalMemory);
        }
        if (processes[i].pageTable) {
            free(processes[i].pageTable);
        }
    }
    // Close input and output files
    fclose(input_file);
    fclose(output_file);

    return 0;
}