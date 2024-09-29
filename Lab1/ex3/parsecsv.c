#include <stdio.h>
#include <string.h>
#include <math.h>
#include "record_list.h"
#include "util.h"

int main(int argc, char** argv) {

    char usage[] = "Usage: parsecsv.out <input CSV file> <output CSV file>\n\n";
    char foerr[] = "Error: unable to open/create file\n\n";


    if ( argc != 3 ) {
        fprintf(stderr, "Usage: parsecsv.out <input CSV file> <output CSV file>\n\n");
        return -1;
    }

    FILE* fin = fopen(argv[1], "r");
    if ( fin == NULL ) {
        fprintf(stderr, "Error: unable to open file %s\n\n", argv[1]);
        return -2;
    }

    int* newline = read_next_line(fin);
    record_t* head = NULL;
    record_t* curr = NULL;
    
    while (newline != NULL) {
        record_t* new_record = (record_t*)malloc(sizeof(record_t));
        if (new_record == NULL) {
            fprintf(stderr, "Error: memory allocation failed\n");
            return -3;
        }
        new_record->avg = compute_average(newline);
        new_record->sdv = compute_stdev(newline);
        new_record->next = NULL;

        if (head == NULL) {
            head = new_record;
        } else {
            curr = head;
            while (curr->next != NULL) {
                curr = curr->next;
            }
            curr->next = new_record;
        }
        free(newline);
        newline = read_next_line(fin);
    }
    fclose(fin);

    // TODO: write the list to the output file
    // Each line of the output file should contain the average and the standard deviation
    // as a comma-separated pair (e.g., "1.23,4.56")
    fin = fopen(argv[2], "w");
    if ( fin == NULL ) {
        fprintf(stderr, "Error: unable to open file %s\n\n", argv[2]);
        return -2;
    }
    curr = head;
    while (curr != NULL) {
        char avg_str[50];
        char sdv_str[50];

        
        sprintf(avg_str, "%f", curr->avg);
        sprintf(sdv_str, "%f", curr->sdv);

        strcat(avg_str,", ");
        strcat(avg_str, sdv_str);
        strcat(avg_str, "\n");

        fprintf(fin, "%s", avg_str);

        curr = curr->next;
    }

    // TODO: free all the memory allocated for the list
    curr = head;
    record_t* prev;
    while(curr != NULL){
        prev = curr;
        curr = curr->next;
        free(prev);
    }

    return 0;
}
