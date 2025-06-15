include ../../common/Makefile

FLAG += -pthread -D_GNU_SOURCE -Wall -Wextra -pthread -std=c17 -Isrc
FLAG += -fopenmp

#ARGS = jobs/job001b/job001.txt
ARGS = jobs/job002b/job002.txt
#ARGS = jobs/job003b/job003.txt 

CC=mpicc
XC=mpic++

RUNPRE=mpiexec -np 3