// Copyright 2024-2025 ECCI-UCR CC-BY-4
#include <iostream>

#include "Mpi.hpp"

#include <sstream>

#define FIRST_PROCESS 0

// 70 chars for "Hello from main thread of process <i> of <w> on <hostname>"
// constexpr indicates the expression does not have address, is constant
constexpr size_t MAX_MSG_LEN = MPI_MAX_PROCESSOR_NAME + 70;

int main(int argc, char* argv[]) {
  try {
    Mpi mpi(argc, argv);
    std::stringstream message;
    message << "Hello from main thread of process " << mpi.rank()
        << " of " << mpi.size() << " on " << mpi.getHostname();
  
    if (mpi.rank() > FIRST_PROCESS) {
      // send msg to first process to print
      mpi.send(message.str(), FIRST_PROCESS);
    } else {
      std::cout << message.str() << std::endl;
      // Iterate through processes (to print in order)
      for (int source = FIRST_PROCESS + 1; source < mpi.size(); ++source) {
        std::string buffer;
        mpi.receive(buffer, MAX_MSG_LEN);  // Receive from any source
        std::cout << buffer << std::endl;  // Print process's msg
      }
    }
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << std::endl;
  }
}
