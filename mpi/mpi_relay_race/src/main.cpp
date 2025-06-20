// Copyright 2024 ECCI-UCR CC-BY-4

#include <unistd.h>
#include <iostream>

#include "Mpi.hpp"

#define FIRST_PROCESS 0

void relay_race(Mpi& mpi, const int stage1_delay, const int stage2_delay);
void run_stage1(Mpi& mpi, int stage1_delay);
void run_stage2(Mpi& mpi, int stage2_delay);
void referee(Mpi& mpi);

int main(int argc, char* argv[]) {
  try {
    Mpi mpi(argc, argv);
    if (mpi.size() >= 3 && mpi.size() % 2 == 1 && argc == 3) {
      relay_race(mpi, atoi(argv[1]), atoi(argv[2]));
    } else if (mpi.rank() == 0) {
      std::cerr << "usage: mpiexec -np n relay_race stage1_msec stage2_msec\n"
        << "n must be odd and greater or equal to 3\n";
    }
  } catch (const Mpi::Error& error) {
    std::cerr << "error: " << error.what() << std::endl;
  }
  return 0;
}

void relay_race(Mpi& mpi, const int stage1_delay, const int stage2_delay) {
  mpi.barrier();  // Procedure that emulates a barrier's functionality
  if (mpi.rank() == FIRST_PROCESS) {
    referee(mpi);  // First process acts like referee to centralize
  } else if (mpi.rank() % 2 == 1) {
    // Processes with odd ids run the first stage
    run_stage1(mpi, stage1_delay);
  } else {
    // Processes with even ids run the second stage
    run_stage2(mpi, stage2_delay);
  }
}

void run_stage1(Mpi& mpi, int stage1_delay) {
  usleep(1000 * stage1_delay);
  const bool baton = true;
  mpi.send(baton, mpi.rank() + 1);  // Give baton to next process (teammate)
}

void run_stage2(Mpi& mpi, int stage2_delay) {
  usleep(1000 * stage2_delay);
  bool baton = false;
  mpi.receive(baton, mpi.rank() - 1);  // Waits for baton from teammate (prev)
  const int team = mpi.rank() / 2;  // Since this is ran by even process
  mpi.send(team, FIRST_PROCESS);
}

void referee(Mpi& mpi) {
  // Amount of teams, with two processes each
  const int teams = (mpi.size() - 1) / 2;
  const double start_time = Mpi::wtime();
  for (int place = 1; place <= teams; ++place) {
    int team = 0;
    mpi.receive(team, MPI_ANY_SOURCE);
    double elapsed = Mpi::wtime() - start_time;
    std::cout << "Place " << place << ": team " << team << " in " << elapsed <<
        std::endl;
  }
}
