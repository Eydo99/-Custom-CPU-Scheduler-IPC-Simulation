#include "headers.h"



int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./process_generator <processes.txt>\n");
        exit(1);
    }

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct process_data);
    attr.mq_curmsgs = 0;

    // TODO 1: Initialize the POSIX Message Queue (mq_open)
    mqd_t mq; 
    mq=mq_open(QUEUE_NAME,O_WRONLY|O_CREAT,0644,&attr);
    if(mq==(mqd_t)-1)
    {
        perror("mq_open");
        exit(1);
    }
        

    FILE *file = fopen(argv[1], "r");
    char buffer[256];
    fgets(buffer, sizeof(buffer), file); // Skip header

    struct process_data p;
    while (fscanf(file, "%d %d %d %d", &p.id, &p.arrival_time, &p.run_time, &p.priority) == 4) {
        // TODO 2: Send the process data to the queue (mq_send)
        printf("Generator: Sent Process %d\n", p.id);
        mq_send(mq,(char*)&p,sizeof(p),0);
    }

    // TODO 3: Send a termination message (Process with ID -1)
   struct process_data dummy={.id=-1};
   mq_send(mq,(char*)&dummy,sizeof(dummy),0);

    
    printf("Generator: All processes sent. Exiting.\n");
    fclose(file);
    // TODO 4: Close the queue (mq_close)
    mq_close(mq);
    return 0;
}