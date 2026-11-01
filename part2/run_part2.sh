#!/bin/bash
REFERENCE_SPEED=50   # rpm, same as Project 4
RUN_TIME=10          # seconds per test

mkdir -p results


# 1. Normal, no extra load
echo "=== normal"
./motor_control $REFERENCE_SPEED $RUN_TIME results/log_normal.csv > results/print_normal.txt
sleep 2


# 2. CPU fully loaded (stress-ng on all 4 cores)
stress-ng --cpu 4 --quiet &
sleep 2

echo "=== load"
./motor_control $REFERENCE_SPEED $RUN_TIME results/log_load.csv > results/print_load.txt
sleep 2


# 3. CPU fully loaded, but the controller gets real-time priority (SCHED_FIFO, 80)
echo "=== load + real-time priority"
chrt -f 80 ./motor_control $REFERENCE_SPEED $RUN_TIME results/log_load_rt.csv > results/print_load_rt.txt

pkill stress-ng


echo
echo "Done. Measurements in results/log_*.csv"
