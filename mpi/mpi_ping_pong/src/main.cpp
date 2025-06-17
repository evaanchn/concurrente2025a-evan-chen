// Copyright 2024-2025 ECCI-UCR CC-BY-4
#include <unistd.h>
#include <iostream>

#include "Mpi.hpp"

#define FIRST_PROCESS 0
void play_ping_pong(Mpi& mpi, const long delay);

int main(int argc, char* argv[]) {
  try {
    Mpi mpi(argc, argv);
    if (mpi.size() == 2) {
      const long delay = argc >= 2 ? std::stol(argv[1]) : 500;  // milliseconds
      play_ping_pong(mpi, delay);
    } else if (mpi.rank() == 0) {
      std::cerr << "Only two processes are allowed" << std::endl;
    }
  } catch (const Mpi::Error& error) {
    std::cerr << "error: " << error.what() << std::endl;
  }
  return 0;
}

void play_ping_pong(Mpi& mpi, const long delay) {
  long ball = 0;
  // Process 0 serves first
  if (mpi.rank() == FIRST_PROCESS) {
    mpi.send(ball, FIRST_PROCESS + 1);  // Send ball to the other process
  }

  while (true) {
    mpi.receive(ball);  // Wait to receive ball
    std::cout << mpi.rank() << " serves " << ball << std::endl;  // Report ball
    ++ball;
    usleep(1000 * delay);
    int otherPlayer = (mpi.rank() + 1) % 2;  // Calculate the other process
    mpi.send(ball, otherPlayer);  // Send to the 
  }
}
