#!/bin/bash

# 1. Clean and Compile the code
echo "======================================"
echo "Compiling the Lab Components..."
echo "======================================"
make clean > /dev/null 2>&1
make

if [ $? -ne 0 ]; then
    echo "Compilation failed! Please check your C code."
    exit 1
fi

# Function to safely run the simulation and rename the logs
run_simulation() {
    ALGO=$1
    QUANTUM=$2
    ALGO_NAME=$3

    echo ""
    echo "--------------------------------------"
    if [ "$ALGO" -eq 2 ]; then
        echo "Starting $ALGO_NAME (Quantum: $QUANTUM)..."
        ./scheduler $ALGO $QUANTUM &
    else
        echo "Starting $ALGO_NAME..."
        ./scheduler $ALGO &
    fi

    SCHED_PID=$!

    # Give the scheduler a second to set up the POSIX Message Queue
    sleep 1 

    ./process_generator processes.txt
    
    # Wait for the Scheduler to finish executing
    wait $SCHED_PID

    # Cleanup Failsafe 
    rm -f /dev/mqueue/scheduler_mq 2>/dev/null

    # Rename the output files so they don't get overwritten by the next run!
    mv scheduler.log scheduler_${ALGO_NAME}.log 2>/dev/null
    mv metrics.txt metrics_${ALGO_NAME}.txt 2>/dev/null
    
    echo ">> $ALGO_NAME Log saved to scheduler_${ALGO_NAME}.log"
}

# 2. Check for the --all flag
if [ "$1" == "--all" ]; then
    echo ""
    echo "======================================"
    echo "Running ALL algorithms for testing..."
    echo "======================================"
    
    run_simulation 1 0 "FCFS"
    run_simulation 2 3 "RR"   # You can change the default testing quantum here
    run_simulation 3 0 "HPF"
    
    echo ""
    echo "======================================"
    echo "Batch Testing Complete!"
    echo "Check the newly generated log files."
    echo "======================================"
    exit 0
fi

# 3. Interactive Mode (If --all is not passed)
echo ""
echo "======================================"
echo "Select Scheduling Algorithm:"
echo "1. First Come First Serve (FCFS)"
echo "2. Round Robin (RR)"
echo "3. Highest Priority First (HPF)"
echo "======================================"
read -p "Enter choice (1, 2, or 3): " ALGO

if [ "$ALGO" -eq 2 ]; then
    read -p "Enter Time Quantum for RR: " QUANTUM
    run_simulation $ALGO $QUANTUM "RR"
elif [ "$ALGO" -eq 1 ]; then
    run_simulation 1 0 "FCFS"
elif [ "$ALGO" -eq 3 ]; then
    run_simulation 3 0 "HPF"
else
    echo "Invalid choice! Exiting."
    exit 1
fi