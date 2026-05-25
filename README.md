# Custom CPU Scheduler & IPC Simulation

A deterministic simulation of an OS process scheduler built in C, using **POSIX Message Queues** for IPC and **UNIX Signals** for process control. The simulation uses **Virtual Time** (Discrete Event Simulation) instead of real-time, ensuring instant execution and reproducible results.

---

## Project Structure

```
.
├── headers.h            # Shared definitions (QUEUE_NAME, process_data struct)
├── process_generator.c  # Reads processes from file and sends them via MQ
├── scheduler.c          # Virtual clock, scheduling algorithms, logging
├── process.c            # Dummy process that waits for signals
├── processes.txt        # Input file with process data
├── Makefile             # Build system
└── runner.sh            # Interactive/batch test runner
```

---

## How It Works

### IPC Flow
1. The **Scheduler** starts first and creates a POSIX Message Queue.
2. The **Process Generator** reads `processes.txt` and sends all processes to the queue, followed by a termination message (`ID = -1`).
3. The **Scheduler** reads all messages, then starts the virtual clock.

### Virtual Time Loop
Each tick of the virtual clock:
1. **Arrivals** — If a process's `arrival_time == current_time`, `fork()` + `execl()` it and immediately `SIGSTOP` it.
2. **Termination** — If the running process's `remaining_time == 0`, kill it with `SIGKILL` and `waitpid()`.
3. **Scheduling** — Pick the next process based on the algorithm, send `SIGCONT`.
4. **Stats Update** — Decrement `remaining_time` for the running process, increment `waiting_time` for waiting ones.

---

## Scheduling Algorithms

| ID | Algorithm | Type | Rule |
|----|-----------|------|------|
| 1 | **FCFS** | Non-Preemptive | Runs processes in order of arrival |
| 2 | **Round Robin** | Preemptive | Each process runs for a time quantum `Q`, then is moved to the back of the queue |
| 3 | **HPF** | Non-Preemptive | Lowest priority number runs first; tie-break by arrival time |

---

## Build & Run

### Compile
```bash
make
```

### Run Interactively
```bash
chmod +x runner.sh
./runner.sh
```
Then select an algorithm (1, 2, or 3). For Round Robin, you will be prompted for a time quantum.

### Run All Algorithms at Once
```bash
./runner.sh --all
```
This runs FCFS, RR (quantum=3), and HPF sequentially and saves each result to a separate log file.

### Clean Build Artifacts
```bash
make clean
```

---

## Input Format

`processes.txt` — one process per line, with a comment header:

```
# ID Arrival_Time Run_Time Priority
1 0 5 3
2 2 3 2
3 4 4 1
4 10 2 4
```

---

## Output Files

### `scheduler.log`
Logs every process state change with virtual clock time:
```
At time 0 process 1 started arr 0 total 5 remain 5 wait 0
At time 3 process 1 stopped arr 0 total 5 remain 2 wait 0
At time 3 process 2 started arr 2 total 3 remain 3 wait 1
At time 6 process 2 finished arr 2 total 3 remain 0 wait 1 TA 4
```

### `metrics.txt`
Summary statistics at the end of the simulation:
```
CPU utilization = 93.33%
Avg TAT = 6.25, Avg WT = 2.75
```

When using `runner.sh`, these files are automatically renamed per algorithm (e.g. `scheduler_FCFS.log`, `metrics_RR.txt`).

---

## Expected Results (on default `processes.txt`)

| Algorithm | CPU Utilization | Avg TAT | Avg WT |
|-----------|----------------|---------|--------|
| FCFS      | 93.33%         | 5.75    | 2.25   |
| RR (Q=3)  | 93.33%         | 6.25    | 2.75   |
| HPF       | 93.33%         | 6.00    | 2.50   |

---

## Key Implementation Details

- **POSIX Message Queue** — created with `mq_open`, sized to fit `struct process_data`. Must be unlinked at the end with `mq_unlink`.
- **Signal Control** — `SIGSTOP` pauses a process, `SIGCONT` resumes it, `SIGKILL` terminates it.
- **Zombie Cleanup** — `waitpid()` is called after every `SIGKILL` to prevent zombie processes.
- **Determinism** — `usleep(1000)` at the end of each tick gives the Linux kernel time to process signals, ensuring consistent behavior.
- **Linker Flag** — `-lrt` is required to link the POSIX real-time library for message queues.

---

## Metrics Definitions

| Metric | Definition |
|--------|-----------|
| **TAT** (Turnaround Time) | `finish_time - arrival_time` |
| **WT** (Waiting Time) | Time spent in the ready queue, not running |
| **CPU Utilization** | `(busy_ticks / total_ticks) * 100` |