#!/bin/bash
# This script runs a performance benchmark across multiple directories.

# --- USAGE ---
# ./perfrun-cficheck.sh <base_cycles>
#
# Arguments:
#   <base_cycles> : An integer used as a base multiplier for the benchmark workload.
#
# Example:
#   ./perfrun-cficheck.sh 1000000
# ---------------

# An example command showing how to run this script with tracing enabled for debugging.
# bash -x perfrun.sh 1000000

# Store the first command-line argument in the NCYCLES variable.
NCYCLES=$1
# Store the current working directory so we can return to it later.
ROOT_DIR=`pwd`

# Define a function to run and time the 'main' executable.
measure_time(){
    # Use the '/usr/bin/time' command to measure the execution time of './main'.
    # The workload is calculated by multiplying the base NCYCLES value.
    # The '--' argument separates options for the 'time' command from the command being timed.
    /usr/bin/time -- ./main $(($NCYCLES * 100))
    /usr/bin/time -- ./main $(($NCYCLES * 1000))
    # A longer, more intensive test run that is currently commented out and disabled.
    # /usr/bin/time -- ./main $(($NCYCLES * 10000))
}

# Loop 5 times through the benchmark directories
for run in {1..5}; do
    echo "=== Run $run ==="
    
    # Loop through a predefined list of benchmark directories.
    for dir in calculator-1 calculator-2 calculator-3 calculator-4 calculator-8 calculator-10 calculator-20 calculator-30 calculator-40 calculator-50; do
        # Change into the target subdirectory from the root directory.
        echo "cd $ROOT_DIR/$dir"
        cd $ROOT_DIR/$dir
        # Call the function to execute the performance measurements.
        measure_time
    done
done

# After the loop has finished, return to the original starting directory.
cd $ROOT_DIR