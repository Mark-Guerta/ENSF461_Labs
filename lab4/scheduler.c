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
    int wait; //time job is spent waiting
    struct job *next;
};

struct jobs{
    struct job* jb;
    struct jobs* next;
};

struct jobs* llhead = NULL;

void addNode(struct jobs* jobs, struct job* job ){
    while(1){
        if(jobs->next == NULL){ 
            jobs->next = (struct jobs*)malloc(sizeof(jobs));
            jobs->next->next = NULL;
            jobs->next->jb = job;
            break;
        }
        jobs = jobs->next;
    }
};

void removeNode(struct jobs* jobs, struct job* job){
    struct jobs* prev = NULL;
    while(jobs != NULL){
        if(jobs->jb->id == job->id){
            if(jobs->next == NULL){ //End Node
                if(prev != NULL){//Not Last Node
                    prev->next = NULL;
                }
                else{ //Very Last Node
                    llhead = NULL;
                }
                free(jobs);
            }
            else if(prev == NULL){ //First Node
                llhead = llhead->next;
                free(jobs);
            }
            else{ //Middle node
                prev->next = jobs->next;
                free(jobs);
            }
            break;
        }
        prev = jobs;
        jobs = jobs->next;
    }
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
    new_job->wait = -1;

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

    while (numofjobs) {
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
            if ((shortest != NULL)&&(shortest->start_time == -1)) {
                shortest->start_time = current_time;
            }
            printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", current->start_time, current->id - 1, current->arrival, current_time - current->start_time);
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
    int time = 0;

    while (numofjobs) {
        struct job *search = head; // Reset search to the head of the list for each time unit
        struct job *shortest_job = NULL;

        while (search) {
            // Job polling (Checks when jobs arrive, time remaining is not 0, and if there are other jobs that take less time)
            
            if (search->arrival <= time && search->end_time == -1) {
                if (shortest_job == NULL || search->remaining_time < shortest_job->remaining_time)
                    shortest_job = search;
            }
            search = search->next;
        }
        if (current != NULL && current != shortest_job &&current->remaining_time > 0){
            printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", current->start_time, current->id - 1, current->arrival, time - current->start_time);
            current->start_time = -1;
        }
        current = shortest_job;

        if (current != NULL && current->start_time == -1)
            current->start_time = time;

        if (current != NULL && current->remaining_time > 0) {
            current->remaining_time--;
        } 
        else if (current != NULL && current->remaining_time == 0) {
            current->end_time = time;
            printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", current->start_time, current->id - 1, current->arrival, current->end_time - current->start_time);
            current = NULL;
            numofjobs--;
            continue;
        }
        time++;
    }

    printf("End of execution with STCF.\n");
}

void policy_RR(int slice) {
    printf("Execution trace with RR:\n");

    struct job *current = head;
    struct job *w = head;
    int current_time = 0;
    int c_time = 0;
    while (numofjobs) {
        current = head;
        int status = 0;
        while ((current != NULL)) {
            if ((current->remaining_time > 0)&&(current->arrival <= current_time)) {
                c_time = current_time;
                if (current->start_time == -1) {
                    current->start_time = current_time;
                }

                if (current->remaining_time > slice) {
                    current->remaining_time -= slice;
                    current_time += slice;
                    status = 1;
                } else {
                    current_time += current->remaining_time;
                    current->remaining_time = 0;
                    current->end_time = current_time;
                    numofjobs--;
                    status = 1;
                }

                while(w){
                    if((w->end_time == -1)&&(w != current)&&(w->arrival <= current_time)){
                        if(w->wait == -1){
                            w->wait = 0;
                        }
                        if((current_time -  w->arrival) >= slice){
                            w->wait += (current_time - c_time);
                        }
                        else{
                            w->wait += (current_time - w->arrival);
                        }
                        
                    }
                    w = w->next;
                }
                w = head;

                printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", c_time, current->id - 1, current->arrival, current_time - c_time);
            }
            current = current->next;
        }
        if(status == 0){
            current_time++;
        }
    }
    while(w){
        if(w->wait == -1){
            w->wait = 0;
        }
        w = w->next;
    }
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

    struct job* temp = head;

    llhead = (struct jobs*)malloc(sizeof(struct jobs));
    llhead->jb = temp;
    llhead->next = NULL;
    struct jobs* lljPointer = llhead;

    int total_tickets = temp->tickets;
    temp = temp->next;

    while(temp){
        total_tickets = total_tickets + temp->tickets;
        addNode(lljPointer, temp);
        lljPointer = llhead;
        temp = temp->next;
    }
    int winning_ticket = rand() % total_tickets;
    int time_taken =  lljPointer->jb->arrival;
    int jobs_remaining = numofjobs;

    while(jobs_remaining){
        int tickets_passed = 0;
        int update_arrival_time = 0;
        lljPointer = llhead;

        while(lljPointer != NULL){ 
            tickets_passed = tickets_passed + lljPointer->jb->tickets;
            if(time_taken < lljPointer->jb->arrival){
                //Skip to next job
                update_arrival_time++;
                if(update_arrival_time == jobs_remaining + 1){
                    time_taken = lljPointer->jb->arrival;
                }
            }
            else if(tickets_passed >= winning_ticket){
                break;
            }
            lljPointer = lljPointer->next;
            if(lljPointer == NULL){
                lljPointer = llhead;
                winning_ticket -= tickets_passed;
                tickets_passed = 0;
            }
        }

        winning_ticket = rand() % total_tickets;

        printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", time_taken, lljPointer->jb->id - 1, lljPointer->jb->arrival, slice);

        if(lljPointer->jb->start_time == -1){
            lljPointer->jb->start_time = time_taken;
        }
        time_taken += slice;
        lljPointer->jb->remaining_time -= slice;

        if(lljPointer->jb->remaining_time <= 0){
            if(lljPointer->jb->end_time == -1){
                lljPointer->jb->end_time = time_taken;
            }
            lljPointer->jb->remaining_time = lljPointer->jb->length;
            lljPointer->jb->wait = time_taken - lljPointer->jb->arrival - lljPointer->jb->remaining_time; 

            jobs_remaining -= 1;
            temp = lljPointer->jb;
            lljPointer = llhead;

            removeNode(lljPointer, temp);

            if(jobs_remaining == 0){
                break;
            }
        }
    }


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
                double avgResponse = 0;
                double avgTurnaround = 0;
                double avgWait = 0;

                while(temp){
                   if (temp->length > temp->end_time - temp->start_time){
                        printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", temp->id - 1, temp->arrival - temp->arrival, temp->end_time - temp->arrival, (temp->end_time - temp->arrival) - temp->length);
                        avgResponse += temp->arrival - temp->arrival; // In STCF, start time is equal to arrival time for an interrupted job
                        avgTurnaround += (temp->end_time - temp->arrival);
                        avgWait += (temp->end_time - temp->arrival) - temp->length;
                        temp = temp->next;
                        i++;
                    }
                   else{
                        printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", temp->id - 1, temp->start_time - temp->arrival, temp->end_time - temp->arrival, temp->start_time - temp->arrival);
                        avgResponse += (temp->start_time - temp->arrival);
                        avgTurnaround += (temp->end_time - temp->arrival);
                        avgWait += (temp->start_time - temp->arrival);
                        temp = temp->next;
                        i++;
                   }
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
        if (analysis == 1){
            printf("Begin analyzing RR:\n");
            {
                struct job* temp = head;
                if(temp == NULL){
                    fprintf(stderr, "Failed to copy head pointer");
                    exit(1);
                }
                int i = 0;
                double avgResponse = 0;
                double avgTurnaround = 0;
                double avgWait = 0;

                while(temp){
                    printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", temp->id - 1, temp->start_time - temp->arrival, temp->end_time - temp->arrival, temp->wait);

                    avgResponse = avgResponse + (temp->start_time - temp->arrival);
                    avgTurnaround = avgTurnaround + (temp->end_time - temp->arrival);
                    avgWait = avgWait + (temp->wait);
                    temp = temp->next;
                    i++;
                }

                avgResponse = avgResponse / i;
                avgTurnaround = avgTurnaround / i;
                avgWait = avgWait / i;
                printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n", avgResponse, avgTurnaround, avgWait);
            }
            printf("End analyzing RR.\n");
        }
    } else if (strcmp(pname, "LT") == 0) {
        policy_LT(slice);
        if (analysis == 1) {
            printf("Begin analyzing LT:\n");

            {
                struct job* temp = head;
                if(temp == NULL){
                    fprintf(stderr, "Failed to copy head pointer");
                    exit(1);
                }
                int i = 0;
                double avgResponse = 0;
                double avgTurnaround = 0;
                double avgWait = 0;

                while(temp){
                    printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", temp->id - 1, temp->start_time - temp->arrival, temp->end_time - temp->arrival, temp->wait);

                    avgResponse = avgResponse + (temp->start_time - temp->arrival);
                    avgTurnaround = avgTurnaround + (temp->end_time - temp->arrival);
                    avgWait = avgWait + (temp->wait);
                    temp = temp->next;
                    i++;
                }

                avgResponse = avgResponse / i;
                avgTurnaround = avgTurnaround / i;
                avgWait = avgWait / i;
                printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n", avgResponse, avgTurnaround, avgWait);
            }
            printf("End analyzing LT.\n");
        }
    } else {
        fprintf(stderr, "Unknown policy: %s\n", pname);
        fprintf(stderr, usage, argv[0]);
        exit(1);
    }

    exit(0);
}