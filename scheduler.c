#include "headers.h"
#include <stdbool.h>

// structure to track process state in the Ready Queue
struct pcb {
    struct process_data data;
    pid_t pid;
    int remaining_time;
    int waiting_time;
    bool is_started;
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./scheduler <algo_id> [quantum]\n");
        exit(1);
    }

    int algo = atoi(argv[1]);
    int quantum = (argc == 3) ? atoi(argv[2]) : 0;

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct process_data);
    attr.mq_curmsgs = 0;
    // TODO 1: Open the POSIX Message Queue
    mqd_t mq;
    mq=mq_open(QUEUE_NAME,O_RDONLY|O_CREAT,0644,&attr);
    if(mq==(mqd_t)-1)
    {
        perror("mq_open");
        exit(1);
    }

    // TODO 2: Read ALL processes from the MQ and store them in a local array
    struct process_data future_processes[100];
    int total_processes = 0;
    // Hint: Read until you receive a process with ID -1

    struct process_data p;
    while(1)
    {
        mq_receive(mq,(char*)&p,sizeof(p),NULL);
        if(p.id==-1)
            break;
        future_processes[total_processes]=p;
        total_processes++;
    }
    
    printf("Scheduler: Received %d processes. Starting Virtual Clock...\n", total_processes);

    int current_time = 0;
    int processes_completed = 0;
    int running_idx = -1; // Index of the process currently on the "CPU"
    struct pcb ready_queue[100]; //my ready queue
    int queue_size = 0;
    int waiting_queue[100];
    int wq_front=0,wq_back=0;
    int quantam_remaining=0;
    int busy_ticks=0; // to track how many ticks cpu was busy
    float total_tat=0,total_wt=0; //to calculate avg tat and avg_wt
    FILE *log_file = fopen("scheduler.log", "w"); //logging file that update changes

    // ==========================================================
    // VIRTUAL TIME LOOP
    // ==========================================================
    while (processes_completed < total_processes) {
        
        // TODO 3: Handle Arrivals
        for(int i=0;i<total_processes;i++)
        {
            // Check future_processes. If any arrived at 'current_time':
            if(future_processes[i].arrival_time==current_time)
            {
                // - fork() and exec() the dummy process
                pid_t pid=fork();
                if(!pid)
                   execl("./process", "process", NULL);
                else{
                        // - kill(pid, SIGSTOP) immediately
                        kill(pid,SIGSTOP); 

                        // - Add to your Ready Queue data structure
                        struct pcb current_pcb={.data=future_processes[i],
                                        .pid=pid,
                                        .remaining_time=future_processes[i].run_time,
                                        .waiting_time=0,
                                        .is_started=false};

                        ready_queue[queue_size]=current_pcb;
                        queue_size++;

                        waiting_queue[wq_back] = queue_size - 1;
                        wq_back++;
                    }
            }
           
        }
        
        // TODO 4: Handle Termination
        // If a process is running and its remaining_time == 0:
        if(running_idx!=-1 && ready_queue[running_idx].remaining_time==0)
        {
            
            pid_t pid=ready_queue[running_idx].pid;
            // - kill(pid, SIGKILL)
            kill(pid,SIGKILL);
            // - waitpid(pid, NULL, 0)
            waitpid(pid,NULL,0);

            // - Log "finished", update metrics, processes_completed++
            int tat = current_time - ready_queue[running_idx].data.arrival_time;
            total_tat+=tat;
            total_wt+=ready_queue[running_idx].waiting_time;

            fprintf(log_file, "At time %d process %d finished arr %d total %d remain 0 wait %d TA %d\n",
            current_time,
            ready_queue[running_idx].data.id,
            ready_queue[running_idx].data.arrival_time,
            ready_queue[running_idx].data.run_time,
            ready_queue[running_idx].waiting_time,
            tat);

            processes_completed++;

            running_idx=-1;  //no process currently on cpu
        }
        
        // TODO 5: Scheduling Logic
        // Pick the next process based on 'algo' (FCFS, RR, or HPF)
        if(algo==2 && running_idx!=-1 && quantam_remaining==0)
        {
            pid_t running_p_pid=ready_queue[running_idx].pid;
            kill(running_p_pid,SIGSTOP);

            waiting_queue[wq_back]=running_idx;
            wq_back++;

            struct pcb curr_pcb=ready_queue[running_idx];
            fprintf(log_file, "At time %d process %d stopped arr %d total %d remain %d wait %d \n",
                    current_time,
                    curr_pcb.data.id,
                    curr_pcb.data.arrival_time,
                    curr_pcb.data.run_time,
                    curr_pcb.remaining_time,
                    curr_pcb.waiting_time);
            
            running_idx=-1;
        }


        if(running_idx==-1 && wq_back>wq_front )
        {
            //FCFS
            if(algo==1)
            {
                running_idx=waiting_queue[wq_front];
                wq_front++;
            
            }
            //RR
            else if(algo==2)
            {
                running_idx=waiting_queue[wq_front];
                wq_front++;
                quantam_remaining=quantum;
            }
            //HPF
            else if(algo ==3)
            {
                //start at front of queue
                int best=wq_front;
                //find the process with lowest number(highest priority)
                for(int i=wq_front;i<wq_back;i++)
                {
                    int a=waiting_queue[i];
                    int b=waiting_queue[best];
                    if(ready_queue[a].data.priority<ready_queue[b].data.priority)
                        best=i;
                    if(ready_queue[a].data.priority==ready_queue[b].data.priority)
                        if(ready_queue[a].data.arrival_time<ready_queue[b].data.arrival_time)
                            best=i;
                }

                // swap the best idx to run with the front of quque
                int temp=waiting_queue[best];
                waiting_queue[best]=waiting_queue[wq_front];
                waiting_queue[wq_front]=temp;

                running_idx=waiting_queue[wq_front];
                wq_front++;
            }

            // - If switching: kill(old_pid, SIGSTOP) and kill(new_pid, SIGCONT) 
            struct pcb curr_pcb = ready_queue[running_idx];
            kill(curr_pcb.pid, SIGCONT);

                // - Log the state change (started, stopped, resumed)
                if(!curr_pcb.is_started)
                {
                    ready_queue[running_idx].is_started=true;
                    fprintf(log_file, "At time %d process %d started arr %d total %d remain %d wait %d \n",
                    current_time,
                    curr_pcb.data.id,
                    curr_pcb.data.arrival_time,
                    curr_pcb.data.run_time,
                    curr_pcb.remaining_time,
                    curr_pcb.waiting_time);
                }
                else
                {
                    fprintf(log_file, "At time %d process %d resumed arr %d total %d remain %d wait %d \n",
                    current_time,
                    curr_pcb.data.id,
                    curr_pcb.data.arrival_time,
                    curr_pcb.data.run_time,
                    curr_pcb.remaining_time,
                    curr_pcb.waiting_time);
                }
        
        }
    
        // TODO 6: Update Virtual Time Stats
        for(int i=0;i<queue_size;i++)
        {
            // - Increment waiting_time for processes in the Ready Queue
            if (i==running_idx)
                continue;
            ready_queue[i].waiting_time++;
        }

        // - Decrement remaining_time for the running process
        if(running_idx!=-1)
        {
            ready_queue[running_idx].remaining_time--;
        }

        //decrement quantum time in case of rr
        if(algo==2 && running_idx!=-1)
            quantam_remaining--;

        //if cpu is busy incremnt the busy ticks
        if(running_idx!=-1)
            busy_ticks++;

        current_time++;
        usleep(1000); // 1ms delay to let the OS process signals
    }
       fclose(log_file);
    // ==========================================================

    // TODO 7: Write metrics.txt and clean up IPC (mq_close, mq_unlink)
    float cpu_utilization= ((float)busy_ticks/current_time)*100;
    float avg_tat= total_tat/processes_completed;
    float avg_wt= total_wt/processes_completed;

    FILE *metrics_file= fopen("metrics.txt","w");
    fprintf(metrics_file,"CPU utilization = %.2f%%\n",cpu_utilization);
    fprintf(metrics_file,"Avg TAT = %.2f, Avg WT = %.2f\n",avg_tat,avg_wt);
    fclose(metrics_file);

    mq_close(mq);
    mq_unlink(QUEUE_NAME);
    
    printf("Simulation Finished.\n");
    return 0;
}