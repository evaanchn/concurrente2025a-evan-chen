#!/bin/bash

#SBATCH --overcommit          # Allow more processes than CPUs per node
#SBATCH -J bin/serial         # Job name
#SBATCH -o jobSerial0J20.%j.txt  # Stdout output file (%j expands to jobId)
#SBATCH -N 1                  # Total number of nodes requested
#SBATCH -n 1                  # Total number of processes requested
#SBATCH -t 02:00:00            # Run time (hh:mm:ss) - 1 hr
#SBATCH -p normal             # Partition to use (default, normal, debug)

perf stat bin/serial jobs/job020/job020.txt
