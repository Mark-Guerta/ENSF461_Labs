#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define min(a,b) (((a)<(b))?(a):(b))

// total jobs
int numofjobs = 0;

struct job {
    // job id is ordered by the arrival; jobs arrived first have smaller job id, always increment by 1
    int id;
    int arrival; // arrival time; safely assume the time unit has the minimal increment of 1
    int length;
    int tickets; // number of tickets for lottery scheduling
    int remaining_time; // remaining time for STCF and RR
    int start_time; // time when the job first starts execution
    int end_time; // time when the job finishes execution
    struct job *next;
};

// the workload list
struct job *head = NULL;

void append_to(struct job **head_pointer, int arrival, int length, int tickets) {
    // Allocate memory for the new job
    struct job *new_job = (struct job *)malloc(sizeof(struct job));
    if (new_job == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        return;
    }

    // Initialize the new job with the provided data
    new_job->arrival = arrival;
    new_job->length = length;
    new_job->tickets = tickets;
    new_job->remaining_time = length;
    new_job->start_time = -1; // Initialize start time to -1 (not started)
    new_job->end_time = -1; // Initialize end time to -1 (not finished)
    new_job->next = NULL;

    // If the list is empty, set the new job as the head
    if (*head_pointer == NULL) {
        new_job->id = 1; // Initialize the first job ID to 1
        *head_pointer = new_job;
    } else {
        // Traverse to the end of the list and append the new job
        struct job *tail = *head_pointer;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        new_job->id = tail->id + 1; // Increment the job ID based on the last job's ID
        tail->next = new_job;
    }
    numofjobs++;
}

void read_job_config(const char* filename) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int tickets  = 0;

    char* delim = ",";
    char *arrival = NULL;
    char *length = NULL;

    // Error checking
    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // If the file is empty, exit with error
    if (getline(&line, &len, fp) == -1) {
        fprintf(stderr, "Error: file is empty\n");
        exit(EXIT_FAILURE);
    }
    rewind(fp);

    while ((read = getline(&line, &len, fp)) != -1) {
        if (line[read-1] == '\n') {
            line[read-1] = 0;
        }
        arrival = strtok(line, delim);
        length = strtok(NULL, delim);
        tickets += 100;

        append_to(&head, atoi(arrival), atoi(length), tickets);
    }

    fclose(fp);
    if (line) free(line);
}

void policy_SJF() {
    printf("Execution trace with SJF:\n");

    struct job *current, *shortest;
    int current_time = 0;

    while (numofjobs > 0) {
        current = head;
        shortest = NULL;

        // Find the shortest job that has arrived
        while (current != NULL) {
            if (current->arrival <= current_time && (shortest == NULL || current->length < shortest->length || (current->length == shortest->length && current->arrival < shortest->arrival))) {
                shortest = current;
            }
            current = current->next;
        }

        if (shortest != NULL) {
            if (shortest->start_time == -1) {
                shortest->start_time = current_time;
            }
            printf("[Job %d] arrived at time [%d] ran for [%d]\n", shortest->id, shortest->arrival, shortest->length);
            current_time += shortest->length;
            shortest->end_time = current_time;
            numofjobs--;

            // Remove the job from the list
            if (shortest == head) {
                head = head->next;
            } else {
                current = head;
                while (current->next != shortest) {
                    current = current->next;
                }
                current->next = shortest->next;
            }
            free(shortest);
        } else {
            // No job is ready to run, increment the current time
            current_time++;
        }
    }

    printf("End of execution with SJF.\n");
}

void policy_STCF() {
    printf("Execution trace with STCF:\n");
    // Requirements: When a job arrives, it is added to the queue. The job with the shortest remaining time is selected to run next.
    struct job *current = NULL;
    struct job *search = head;
    int currentTime = 0;
    while(numofjobs > 0){
        while(search != NULL){
            if (search->arrival <= currentTime && search->arrival > 0 && current == NULL){
                current = search;
                current->start_time = currentTime;
            }
            else if(search->arrival <= currentTime && search->arrival > 0 && search->remaining_time < current->remaining_time){
                current = search;
                current->start_time = currentTime;
            }
            search = search->next;
        }
        if (current == NULL)
            continue;
        else if (current->remaining_time > 0 && current->arrival <= currentTime)
            current->remaining_time--;
        else if (current->remaining_time == 0){
            current->remaining_time = 0;
            current->end_time = currentTime;
            numofjobs--;
            current->length = current->end_time - current->start_time;
            printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", currentTime, current->id - 1, current->arrival, current->length);
            current = NULL;
        }
        search = head;
        currentTime++;
    }
    // TODO: implement STCF policy

    printf("End of execution with STCF.\n");
}

void policy_RR(int slice) {
    printf("Execution trace with RR:\n");

    // TODO: implement RR policy

    printf("End of execution with RR.\n");
}

void policy_LT(int slice) {
    printf("Execution trace with LT:\n");

    // Leave this here, it will ensure the scheduling behavior remains deterministic
    srand(42);

    // In the following, you'll need to:
    // Figure out which active job to run first
    // Pick the job with the shortest remaining time
    // Considers jobs in order of arrival, so implicitly breaks ties by choosing the job with the lowest ID

    // To achieve consistency with the tests, you are encouraged to choose the winning ticket as follows:
    // int winning_ticket = rand() % total_tickets;
    // And pick the winning job using the linked list approach discussed in class, or equivalent

    printf("End of execution with LT.\n");
}

void policy_FIFO(){
    printf("Execution trace with FIFO:\n");

    struct job* temp = head;
    if(temp == NULL){
        fprintf(stderr, "Failed to copy head pointer");
        exit(1);
    }

    int total = 0;
    
    while(temp){
        if(temp->arrival > 0 && temp->id - 1 == 0){
            total = temp->arrival;
        }
        printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", total, temp->id - 1, temp->arrival, temp->length);
        total = temp->length + total;
        if(temp->next){
            if(temp->next->arrival > total){
                total = temp->next->arrival;
            }
        }
        temp = temp->next;
    }
    printf("End of execution with FIFO.\n");

    return;
}

int main(int argc, char **argv) {
    static char usage[] = "usage: %s analysis policy slice trace\n";

    int analysis;
    char *pname;
    char *tname;
    int slice;

    if (argc < 5) {
        fprintf(stderr, "missing variables\n");
        fprintf(stderr, usage, argv[0]);
        exit(1);
    }

    // if 0, we don't analyze the performance
    analysis = atoi(argv[1]);

    // policy name
    pname = argv[2];

    // time slice, only valid for RR and LT
    slice = atoi(argv[3]);

    // workload trace
    tname = argv[4];

    read_job_config(tname);

    if (strcmp(pname, "FIFO") == 0) {
        policy_FIFO();
        if (analysis == 1){
            printf("Begin analyzing FIFO:\n");
            {
                struct job* temp = head;
                if(temp == NULL){
                    fprintf(stderr, "Failed to copy head pointer");
                    exit(1);
                }
                int i = 0;
                int total = 0;
                double avgResponse = 0;
                double avgTurnaround = 0;
                double avgWait = 0;

                while(temp){
                    if(temp->arrival > 0 && temp->id - 1 == 0){
                        total = temp->arrival;
                    }
                    printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", temp->id - 1, total - temp->arrival, temp->length + total - temp->arrival, total - temp->arrival);
                    avgResponse = avgResponse + (total - temp->arrival);
                    avgTurnaround = avgTurnaround + (temp->length + total - temp->arrival);
                    avgWait = avgWait + (total - temp->arrival);
                    total = total + temp->length;
                    if(temp->next){
                        if(temp->next->arrival > total){
                            total = temp->next->arrival;
                        }
                    }
                    temp = temp->next;
                    i++;
                }

                avgResponse = avgResponse / i;
                avgTurnaround = avgTurnaround / i;
                avgWait = avgWait / i;
                printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n", avgResponse, avgTurnaround, avgWait);
            }
            printf("End analyzing FIFO.\n");
        }
    } else if (strcmp(pname, "SJF") == 0) {
        policy_SJF();
        if (analysis == 1) {
            // TODO: perform analysis
        }
    } else if (strcmp(pname, "STCF") == 0) {
        policy_STCF();
        if (analysis == 1) {
            printf("Begin analyzing STCF:\n");
            {
                struct job* temp = head;
                if(temp == NULL){
                    fprintf(stderr, "Failed to copy head pointer");
                    exit(1);
                }
                int i = 0;
                int total = 0;
                double avgResponse = 0;
                double avgTurnaround = 0;
                double avgWait = 0;

                while(temp){
                    if(temp->arrival > 0 && temp->id - 1 == 0){
                        total = temp->arrival;
                    }
                    printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", temp->id - 1, total - temp->arrival, temp->length + total - temp->arrival, total - temp->arrival);
                    avgResponse = avgResponse + (total - temp->arrival);
                    avgTurnaround = avgTurnaround + (temp->length + total - temp->arrival);
                    avgWait = avgWait + (total - temp->arrival);
                    total = total + temp->length;
                    if(temp->next){
                        if(temp->next->arrival > total){
                            total = temp->next->arrival;
                        }
                    }
                    temp = temp->next;
                    i++;
                }

                avgResponse = avgResponse / i;
                avgTurnaround = avgTurnaround / i;
                avgWait = avgWait / i;
                printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n", avgResponse, avgTurnaround, avgWait);
            }
            printf("End analyzing STCF.\n");
        }
    } else if (strcmp(pname, "RR") == 0) {
        policy_RR(slice);
        if (analysis == 1) {
            // TODO: perform analysis
        }
    } else if (strcmp(pname, "LT") == 0) {
        policy_LT(slice);
        if (analysis == 1) {
            // TODO: perform analysis
        }
    } else {
        fprintf(stderr, "Unknown policy: %s\n", pname);
        fprintf(stderr, usage, argv[0]);
        exit(1);
    }

    exit(0);
}