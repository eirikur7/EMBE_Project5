#!/bin/bash
# Runs every Part 1 test, one after another, and saves the results in the results/ folder.

mkdir -p results
rm -f results/summary.txt


# Runs one test.
#   $1 = name of the test (used in the file names)
#   $2 = the copy program to start, or "" when the kernel module does the copying
#   $3 = how many edges to test (default 1000)
run_one_test() {
    TEST_NAME=$1
    COPY_PROGRAM=$2
    NUMBER_OF_TESTS=${3:-1000}

    echo "=== $TEST_NAME" | tee -a results/summary.txt

    # Start the copy program in the background
    if [ -n "$COPY_PROGRAM" ]; then
        $COPY_PROGRAM > /dev/null &
        COPY_PID=$!
        sleep 1
    fi

    # Save what top shows during the test. Read the SECOND screen in the file.
    top -b -d 5 -n 2 > results/top_$TEST_NAME.txt &
    TOP_PID=$!

    # Measure. Highest priority, so the measuring itself is not delayed.
    chrt -f 99 ./response_test $NUMBER_OF_TESTS results/delays_$TEST_NAME.txt | tee -a results/summary.txt

    wait $TOP_PID

    # Stop the copy program
    if [ -n "$COPY_PROGRAM" ]; then
        kill $COPY_PID
        wait $COPY_PID 2>/dev/null
    fi
}


# 1. Normal, no extra load
run_one_test shell_idle   "./copy.sh"          200
run_one_test poll_idle    "./gpio_poll 100"
run_one_test events_idle  "./gpio_events"


# 2. CPU fully loaded (stress-ng on all 4 cores)
stress-ng --cpu 4 --quiet &
sleep 2

run_one_test shell_load   "./copy.sh"          200
run_one_test poll_load    "./gpio_poll 100"
run_one_test events_load  "./gpio_events"

# 3. CPU fully loaded, but the copy program gets real-time priority (SCHED_FIFO, 80)
run_one_test shell_load_rt   "chrt -f 80 ./copy.sh"          200
run_one_test poll_load_rt    "chrt -f 80 ./gpio_poll 100"
run_one_test events_load_rt  "chrt -f 80 ./gpio_events"

pkill stress-ng
sleep 2


# 4. Kernel module (no priority to set, the interrupt runs before any program)
insmod lkm/gpio_copy.ko
sleep 1

run_one_test lkm_idle ""

stress-ng --cpu 4 --quiet &
sleep 2
run_one_test lkm_load ""
pkill stress-ng

rmmod gpio_copy


echo
echo "Done. Summary in results/summary.txt"
