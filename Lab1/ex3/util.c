#include "util.h"
#include <stdio.h>
#include <math.h>

int* read_next_line(FILE* fin) {
    if (fin == NULL) {
        return NULL;
    }

    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), fin) == NULL) {
        return NULL;
    }

    int* result = (int*)malloc(sizeof(int) * 1024);
    if (result == NULL) {
        return NULL;
    }

    int count = 0;
    char* token = strtok(buffer, ",");
    while (token != NULL) {
        result[count + 1] = atoi(token);
        count++;
        token = strtok(NULL, ",");
    }

    result[0] = count;
    result = (int*)realloc(result, sizeof(int) * (count + 1));
    return result;
}


float compute_average(int* line) {
    // TODO: Compute the average of the integers in the vector
    // Recall that the first element of the vector is the number of integers
    int len = line[0];
    int i = 1;
    int total;
    float ave;
    while(i != (len+1)){
        total += line[i];
        i++;
    }
    ave = ((float)total)/len;
    return ave;
}


float compute_stdev(int* line) {
    // TODO: Compute the standard deviation of the integers in the vector
    // Recall that the first element of the vector is the number of integers
    int len = line[0];
    float ave = compute_average(line);
    float to_root;
    float temp;
    int i = 1;
    
    while(i != (len+1)){
        temp = (line[i] - ave);
        
        to_root += (temp*temp);
        i++;
    }
    
    to_root = sqrt(to_root/len);
    
    return to_root;
}