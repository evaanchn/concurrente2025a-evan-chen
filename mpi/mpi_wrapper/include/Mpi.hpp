// Copyright 2024 ECCI-UCR CC-BY-4
#pragma once

#include <mpi.h>
#include <string>
#include <stdexcept>

class Mpi {
 private:
  int processNumber = -1;
  int processCount = -1;
  std::string hostName;

 public:
  /// @brief Constructor
  /// @param argc Reference to argument count
  /// @param argv Pointer to arguments vector
  Mpi(int& argc, char**argv) {
    // Initialize MPI
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
      throw std::runtime_error("could not init MPI");
    }
    // Set process number
    if (MPI_Comm_rank(MPI_COMM_WORLD, &this->processNumber) != MPI_SUCCESS) {
      throw std::runtime_error("could not get MPI rank");
    }
    // Set process count
    if (MPI_Comm_size(MPI_COMM_WORLD, &this->processCount) != MPI_SUCCESS) {
      throw std::runtime_error("could not get MPI size");
    }
    // Set host name
    char processHostname[MPI_MAX_PROCESSOR_NAME] = { '\0' };
    int hostnameLen = -1;
    if (MPI_Get_processor_name(processHostname, &hostnameLen)
        != MPI_SUCCESS) {
      throw std::runtime_error("could not get MPI processor name");
    }

    this->hostName = processHostname;
  }

  Mpi(const Mpi&) = delete;  // Disable constructor by copy
  Mpi(Mpi&&) = delete;  // Disable constructor by transfer

  /// @brief Destructor, which calls finalize
  ~Mpi() {
    MPI_Finalize();
  }

  Mpi& operator=(const Mpi&) = delete;  // Disable copy operator
  Mpi& operator=(const Mpi&&) = delete;  // Disable transfer operator

 public:  // Accessors
  /// @brief Process number getter
  /// @return process number
  inline int getProcessNumber() const {
    return this->processNumber;
  }

  /// @brief Process count getter
  /// @return process count
  inline int getProcessCount() const {
    return this->processCount;
  }

  /// @brief Host name getter
  /// @return host name
  inline const std::string& getHostname() const {
    return this->hostName;
  }

  /// @brief Rank or Process number getter
  /// @return process number
  inline int rank() const {
    return this->getProcessNumber();
  }

  /// @brief Size or Process count getter
  /// @return process count
  inline int size() const {
    return this->getProcessCount();
  }
};
