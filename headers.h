#ifndef HEADERS_H
#define HEADERS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>      // For O_* constants
#include <sys/stat.h>   // For mode constants
#include <mqueue.h>     // POSIX Message Queues
#include <string.h>

// POSIX Message Queues require a name starting with a slash
#define QUEUE_NAME "/scheduler_mq"

// Information about each process sent from the generator
struct process_data {
    int id;
    int arrival_time;
    int run_time;
    int priority;
};

#endif