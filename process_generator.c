#include "headers.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./process_generator <processes.txt>\n");
        exit(1);
    }

    // TODO 1: Initialize the POSIX Message Queue (mq_open)
    // YOUR CODE HERE
    mqd_t mq; 

    FILE *file = fopen(argv[1], "r");
    char buffer[256];
    fgets(buffer, sizeof(buffer), file); // Skip header

    struct process_data p;
    while (fscanf(file, "%d %d %d %d", &p.id, &p.arrival_time, &p.run_time, &p.priority) == 4) {
        // TODO 2: Send the process data to the queue (mq_send)
        // YOUR CODE HERE
        printf("Generator: Sent Process %d\n", p.id);
    }

    // TODO 3: Send a termination message (Process with ID -1)
    // YOUR CODE HERE
    
    printf("Generator: All processes sent. Exiting.\n");
    fclose(file);
    // TODO 4: Close the queue (mq_close)
    return 0;
}