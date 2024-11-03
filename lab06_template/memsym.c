#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>

#define TRUE 1
#define FALSE 0
#define MAX_PROCESSES 4
#define TLB_NUM 8
#define PAGENUM 256

// Output file
FILE* output_file;

// TLB replacement strategy (FIFO or LRU)
char* strategy;
uint32_t time = 0;

typedef struct {
    uint32_t VPN;  // Virtual Page Number
    uint32_t PFN;  // Physical Frame Number
    int valid;     // Valid bit
    int process;
    uint32_t time_stamp;
    uint32_t used;
} TLB_entry;

typedef struct {
    uint32_t VPN;  // Virtual Page Number
    uint32_t PFN;  // Physical Frame Number
    int valid;     // Valid bit
} PageTable_entry;

typedef struct {
    PageTable_entry* pageTable;
    uint32_t* physicalMemory;
    int PFN;
    int VPN;
    int OFF; 
    uint32_t R1;
    uint32_t R2;

} Process;

Process processes[MAX_PROCESSES];
int current_process = 0;


TLB_entry TLB[TLB_NUM];

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


void fifo(int VPN, int PFN){
    int lowest = 0;
    for(int i = 1; i<TLB_size; i++){
        if(TLB[i].time_stamp < TLB[lowest].time_stamp){
            TLB[i].VPN = VPN;
            TLB[i].PFN = PFN;
            TLB[i].valid = TRUE;
            TLB[i].process = current_process;
            TLB[i].time_stamp = time;
            TLB[i].used = time;
            return;
        }
    }
}

void lru(int VPN, int PFN){
    int lowest = 0;
    for(int i = 1; i<TLB_size; i++){
        if(TLB[i].used < TLB[lowest].used){
            TLB[i].VPN = VPN;
            TLB[i].PFN = PFN;
            TLB[i].valid = TRUE;
            TLB[i].process = current_process;
            TLB[i].time_stamp = time;
            TLB[i].used = time;
            return;
        }
    }

}

int lookup_pageTable(int VPN){
    for(int i = 0; i<PAGENUM; i++){
        if(processes[current_process].pageTable[i].VPN == VPN){
            return processes[current_process].pageTable[i].PFN;
        }
    }

    return -1;
}

void log_TLB(int m_VPN, int m_PFN){
    for(int i = 0; i<TLB_size; i++){
        if((TLB[i].VPN == m_VPN)&&(TLB[i].process == current_process)){
            TLB[i].PFN = m_PFN;
            TLB[i].valid = TRUE;
            TLB[i].time_stamp = time;
            TLB[i].used = time;
            return;
        }
    }

    for(int i = 0; i<TLB_size; i++){
        if(TLB[i].valid == FALSE){
            TLB[i].VPN = m_VPN;
            TLB[i].PFN = m_PFN;
            TLB[i].valid = TRUE;
            TLB[i].process = current_process;
            TLB[i].time_stamp = time;
            TLB[i].used = time;
            return;
        }
    }

    if(strcmp("FIFO",strategy) == 0){
        fifo(m_VPN, m_PFN);
        return ;
    }else if(strcmp("LRU",strategy) == 0){
        lru(m_VPN, m_PFN);
        return;
    }


}

int lookup_TLB(int VPN){
    for(int i = 0; i < TLB_NUM; i++){
        if(TLB[i].VPN == VPN && TLB[i].valid && TLB[i].process == current_process){
            TLB[i].used = time;
            fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d hit in TLB entry %d. PFN is %d\n", current_process, VPN, i, TLB[i].PFN);
            fflush(output_file);
            return TLB[i].PFN;
        }
    }
    
    // TLB MISS
    fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d missed in TLB\n", current_process, VPN);
    fflush(output_file);

    int PFN = lookup_pageTable(VPN);
    if (PFN == -1) {
        fprintf(output_file, "Current PID: %d. Error: VPN %d not found in page table\n", current_process, VPN);
        fflush(output_file);
        return -1;
    }

    log_TLB(VPN, PFN);
    return PFN;
}

void init_process(int pid, int VPN) {
    int pageTable_size = PAGENUM;
    processes[pid].pageTable = (PageTable_entry*)malloc(pageTable_size * sizeof(PageTable_entry));
    for (int i = 0; i < pageTable_size; i++) {
        processes[pid].pageTable[i].VPN = -1;
        processes[pid].pageTable[i].PFN = -1;  // Invalid PFN
        processes[pid].pageTable[i].valid = FALSE;
    }

}

void handle_Define(int OFF, int PFN, int VPN) {
    int length = 1 << (OFF + PFN);
    for(int i = 0; i < MAX_PROCESSES; i++){
        processes[current_process].physicalMemory = (uint32_t*)calloc(length, sizeof(uint32_t));
    }
    
    init_process(current_process, VPN);
}

void handle_ctxswitch(int pid) {
    if (pid < 0 || pid >= MAX_PROCESSES) {
        fprintf(output_file, "Current PID: %d. Invalid context switch to process %d\n", current_process, pid);
        fflush(output_file);
        return;
    }
    current_process = pid;
    fprintf(output_file, "Current PID: %d. Switched execution context to process: %d\n", current_process, pid);
    fflush(output_file);
}

void handle_map(int m_VPN, int m_PFN){

    for(int i = 0; i<PAGENUM; i++){
        if (processes[current_process].pageTable[i].VPN == m_VPN){
            processes[current_process].pageTable[i].valid = TRUE;
            processes[current_process].pageTable[i].PFN = m_PFN;
            log_TLB( m_VPN, m_PFN);
            fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", current_process, m_VPN, m_PFN);
            fflush(output_file);
            return;
        }
    }
    for(int i = 0; i<PAGENUM; i++){
        if (processes[current_process].pageTable[i].valid == FALSE){
            processes[current_process].pageTable[i].valid = TRUE;
            processes[current_process].pageTable[i].VPN = m_VPN;
            processes[current_process].pageTable[i].PFN = m_PFN;
            log_TLB( m_VPN, m_PFN);
            fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", current_process, m_VPN, m_PFN);
            fflush(output_file);
            return;
        }
    }

   
    return;
}

void remove_LTB(int VPN){
    for(int i = 0; i<TLB_NUM; i++){
        if(TLB[i].VPN == VPN){
            TLB[i].VPN = -1;
            TLB[i].PFN = -1;
            TLB[i].time_stamp = -1;
            TLB[i].used = -1;
            TLB[i].valid = FALSE;
        }
    }
}


void handle_unmap(int m_VPN){
    processes[current_process].pageTable[m_VPN].VPN = -1;
    processes[current_process].pageTable[m_VPN].PFN = -1;
    processes[current_process].pageTable[m_VPN].valid = FALSE;
    fprintf(output_file, "Current PID: %d. Unmapped virtual page number %d\n", current_process, m_VPN);
    fflush(output_file);

    remove_LTB(m_VPN);
    return;
}

void handle_pinspect(int m_VPN){
    int pfn = processes[current_process].pageTable[m_VPN].PFN; 
    int vpn = processes[current_process].pageTable[m_VPN].VPN;
    int valid = processes[current_process].pageTable[m_VPN].valid;
    fprintf(output_file, "Current PID: %d. Inspected page table entry %d. Physical frame number: %d. Valid: %d\n", current_process, vpn, pfn, valid);
    fflush(output_file);
  
}

void handle_tinspect(int m_TLB){
    fprintf(output_file, "Current PID: %d. Inspected TLB entry %d. VPN: %d. PFN: %d. Valid: %d. PID: %d. Timestamp: %d\n", current_process, m_TLB, TLB[m_TLB].VPN, TLB[m_TLB].PFN, TLB[m_TLB].valid, TLB[m_TLB].process, TLB[m_TLB].time_stamp );
    fflush(output_file);
}

int load_from_memory(uint32_t physical_address) {
    return processes[current_process].physicalMemory[physical_address];
}

void store_to_memory(uint32_t physical_address, int immediate_value){
    processes[current_process].physicalMemory[physical_address] = immediate_value;
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
    

    
    int defineCall = 0;
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
        
        time++;

        if (strcmp(tokens[0], "%") == 0) {
           continue;
        }else if(strcmp(tokens[0], "define") == 0) {
            if(defineCall == 1){
                fprintf(output_file,"Current PID: %d. Error: multiple calls to define in the same trace\n", current_process);
                fflush(output_file);
                return -1;
            }
            int OFF = atoi(tokens[1]);
            int PFN = atoi(tokens[2]);
            int VPN = atoi(tokens[3]);
            handle_Define(OFF, PFN, VPN);
            defineCall = 1;
            processes[current_process].VPN = VPN;
            processes[current_process].OFF = OFF;
            processes[current_process].PFN = PFN;
            fprintf(output_file, "Current PID: %d. Memory instantiation complete. OFF bits: %d. PFN bits: %d. VPN bits: %d\n", current_process,OFF, PFN, VPN);
            fflush(output_file);
        }
        else if (strcmp(tokens[0], "ctxswitch") == 0) {
            int pid = atoi(tokens[1]);
            handle_ctxswitch(pid);
        }
        else if (defineCall == 0) {
            fprintf(output_file,"Current PID: %d. Error: attempt to execute instruction before define\n", current_process);
            fflush(output_file);
            return -1;
        }else if (strcmp(tokens[0], "map") == 0) {
            int m_VPN = atoi(tokens[1]);
            int m_PFN = atoi(tokens[2]);
            handle_map(m_VPN, m_PFN);
            log_TLB(m_VPN, m_PFN);
        }else if (strcmp(tokens[0], "unmap") == 0) {
            int m_VPN = atoi(tokens[1]);
            handle_unmap(m_VPN);
            //Dont know if I should delete TLB entry if it exists?
        }else if (strcmp(tokens[0], "pinspect") == 0) {
            int m_VPN = atoi(tokens[1]);
            handle_pinspect(m_VPN);
        }else if (strcmp(tokens[0], "tinspect") == 0) {
            int m_TLB = atoi(tokens[1]);
            handle_tinspect(m_TLB);
        }else if (strcmp(tokens[0], "rinspect") == 0) {
            if(strcmp("r1", tokens[1]) == 0){
                fprintf(output_file, "Current PID: %d. Inspected register r1. Content: %d\n", current_process, processes[current_process].R1);
                fflush(output_file);
            }else if(strcmp(tokens[1], "r2") == 0){
                fprintf(output_file, "Current PID: %d. Inspected register r2. Content: %d\n", current_process, processes[current_process].R2);
                fflush(output_file);
            }else{
                fprintf(output_file, "Current PID: %d. Error: invalid register operand %s\n", current_process, tokens[1]);
                fflush(output_file);
            }
        }else if (strcmp(tokens[0], "load") == 0) {
            if (tokens[2][0] == '#') {
                // Immediate value
                int immediate_value = atoi(tokens[2] + 1); // Skip the '#' character
                // Use the immediate value directly
                if (strcmp(tokens[1], "r1") == 0){
                    processes[current_process].R1 = immediate_value;
                    fprintf(output_file, "Current PID: %d. Loaded immediate %d into register r1\n", current_process, immediate_value);
                    fflush(output_file);
                }else if(strcmp(tokens[1], "r2") == 0){
                    processes[current_process].R2 = immediate_value;
                    fprintf(output_file, "Current PID: %d. Loaded immediate %d into register r2\n", current_process, immediate_value);
                    fflush(output_file);
                }else{
                    fprintf(output_file, "Current PID: %d. Error: invalid register operand %s\n", current_process, tokens[1]);
                    fflush(output_file);
                }
            } else {
                // Memory load
                int virtual_address = atoi(tokens[2]);
                int offset = virtual_address & ((1 << processes[current_process].OFF) - 1); // Extract offset bits
                int VPN = virtual_address >> processes[current_process].OFF; // Extract VPN

                // Find the PFN for the given VPN
                int PFN = lookup_TLB(VPN);
                if (PFN == -1) {
                    fprintf(output_file, "Current PID: %d. Error: invalid virtual address %d\n", current_process, virtual_address);
                    fflush(output_file);
                    continue;
                }

                // Calculate the physical address
                int physical_address = (PFN << processes[current_process].OFF) | offset;

                // Perform the memory load
                uint32_t value = load_from_memory(physical_address);

                if (strcmp(tokens[1], "r1") == 0){
                    processes[current_process].R1 = value;
                    fprintf(output_file, "Current PID: %d. Loaded value of location %d (%d) into register r1\n", current_process, virtual_address, value);
                    fflush(output_file);
                }else if(strcmp(tokens[1], "r2") == 0){
                    processes[current_process].R2 = value;
                    fprintf(output_file, "Current PID: %d. Loaded value of location %d (%d) into register r2\n", current_process, virtual_address, value);
                    fflush(output_file);
                }else{
                    fprintf(output_file, "Current PID: %d. Error: invalid register operand %s\n", current_process, tokens[1]);
                    fflush(output_file);
                }
            }
        }else if (strcmp(tokens[0], "store") == 0) {
            int virtual_address = atoi(tokens[1]);
            int offset = virtual_address & ((1 << processes[current_process].OFF) - 1); // Extract offset bits
            int VPN = virtual_address >> processes[current_process].OFF; // Extract VPN
        
            // Find the PFN for the given VPN
            int PFN = lookup_TLB(VPN);
            if (PFN == -1) {
                fprintf(output_file, "Current PID: %d. Error: invalid virtual address %d\n", current_process, virtual_address);
                fflush(output_file);
                continue;
            }
        
            // Calculate the physical address
            int physical_address = (PFN << processes[current_process].OFF) | offset;
        
            if (tokens[2][0] == '#') {
                // Immediate value
                int immediate_value = atoi(tokens[2] + 1); // Skip the '#' character
                store_to_memory(physical_address, immediate_value);
                // Use the immediate value directly
                fprintf(output_file, "Current PID: %d. Stored immediate %d into location %s\n", current_process, immediate_value, tokens[1]);
                fflush(output_file);
            } else {
                int s_val;
                if (strcmp(tokens[2], "r1") == 0){
                    s_val = processes[current_process].R1;
                }else if(strcmp(tokens[2], "r2") == 0){
                    s_val = processes[current_process].R2;
                }else{
                    fprintf(output_file, "Current PID: %d. Error: invalid register operand %s\n", current_process, tokens[1]);
                    fflush(output_file);
                    continue;
                }

                store_to_memory(physical_address, s_val);

                fprintf(output_file, "Current PID: %d. Stored value of register %s (%d) into location %d\n", current_process, tokens[2], s_val, virtual_address);
                fflush(output_file);
            }
        }else if (strcmp(tokens[0], "add") == 0) {
            int r1 = processes[current_process].R1;
            processes[current_process].R1 += processes[current_process].R2;
            fprintf(output_file, "Current PID: %d. Added contents of registers r1 (%d) and r2 (%d). Result: %d\n", current_process, r1, processes[current_process].R2, processes[current_process].R1);
            fflush(output_file);
        }


        // Deallocate tokens
        for (int i = 0; tokens[i] != NULL; i++)
            free(tokens[i]);
        free(tokens);
    }
    for (int i = 0; i < MAX_PROCESSES; i++) {
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