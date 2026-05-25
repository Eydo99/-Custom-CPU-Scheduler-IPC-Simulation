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

    // TODO 1: Open the POSIX Message Queue
    mqd_t mq;

    // TODO 2: Read ALL processes from the MQ and store them in a local array
    struct process_data future_processes[100];
    int total_processes = 0;
    // Hint: Read until you receive a process with ID -1
    
    printf("Scheduler: Received %d processes. Starting Virtual Clock...\n", total_processes);

    int current_time = 0;
    int processes_completed = 0;
    int running_idx = -1; // Index of the process currently on the "CPU"

    // ==========================================================
    // VIRTUAL TIME LOOP
    // ==========================================================
    while (processes_completed < total_processes) {
        
        // TODO 3: Handle Arrivals
        // Check future_processes. If any arrived at 'current_time':
        // - fork() and exec() the dummy process
        // - kill(pid, SIGSTOP) immediately
        // - Add to your Ready Queue data structure

        // TODO 4: Handle Termination
        // If a process is running and its remaining_time == 0:
        // - kill(pid, SIGKILL)
        // - waitpid(pid, NULL, 0)
        // - Log "finished", update metrics, processes_completed++

        // TODO 5: Scheduling Logic
        // Pick the next process based on 'algo' (FCFS, RR, or HPF)
        // - If switching: kill(old_pid, SIGSTOP) and kill(new_pid, SIGCONT)
        // - Log the state change (started, stopped, resumed)

        // TODO 6: Update Virtual Time Stats
        // - Decrement remaining_time for the running process
        // - Increment waiting_time for processes in the Ready Queue

        current_time++;
        usleep(1000); // 1ms delay to let the OS process signals
    }
    // ==========================================================

    // TODO 7: Write metrics.txt and clean up IPC (mq_close, mq_unlink)
    
    printf("Simulation Finished.\n");
    return 0;
}