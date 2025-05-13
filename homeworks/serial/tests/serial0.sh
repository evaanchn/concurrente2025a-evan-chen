#!/bin/bash

#SBATCH --overcommit          # Allow more processes than CPUs per node
#SBATCH -J bin/serial         # Job name
#SBATCH -o jobSerial0.%j.txt  # Stdout output file (%j expands to jobId)
#SBATCH -N 1                  # Total number of nodes requested
#SBATCH -n 1                  # Total number of processes requested
#SBATCH -t 0:10:00            # Run time (hh:mm:ss) - 10 min
#SBATCH -p normal             # Partition to use (default, normal, debug)

perf stat bin/serial jobs/job003b/job003.txt