// Copyright 2024 ECCI-UCR CC-BY-4
#pragma once

#include <mpi.h>
#include <string>
#include <vector>
#include <stdexcept>

class Mpi {
 private:
  int processNumber = -1;
  int processCount = -1;
  std::string hostName;

 public:
  class Error: public std::runtime_error {
   public:
    explicit Error(const std::string& message)
      : runtime_error(message) {
    }
    Error(const std::string& message, const Mpi& mpi)
      : runtime_error(mpi.getHostname() + ':' + std::to_string(mpi.rank())
      + ':' + message) {}
    Error(const std::string& message, const Mpi& mpi, const int threadNumber)
      : runtime_error(mpi.getHostname() + ':' + std::to_string(mpi.rank())
      + '.' + std::to_string(threadNumber) + ':' + message) {}
  };

 public:
  /// @brief Constructor
  /// @param argc Reference to argument count
  /// @param argv Pointer to arguments vector
  Mpi(int& argc, char**argv) {
    // Initialize MPI
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
      throw Error("could not init MPI");
    }
    // Set process number
    if (MPI_Comm_rank(MPI_COMM_WORLD, &this->processNumber) != MPI_SUCCESS) {
      throw Error("could not get MPI rank");
    }
    // Set process count
    if (MPI_Comm_size(MPI_COMM_WORLD, &this->processCount) != MPI_SUCCESS) {
      throw Error("could not get MPI size");
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

 public:  // Mapping types. Invoke with Mpi::func()
  static inline MPI_Datatype map(bool) { return MPI_C_BOOL; }
  static inline MPI_Datatype map(char) { return MPI_CHAR; }
  static inline MPI_Datatype map(unsigned char) { return MPI_UNSIGNED_CHAR; }
  static inline MPI_Datatype map(short) { return MPI_SHORT; }
  static inline MPI_Datatype map(unsigned short) { return MPI_UNSIGNED_SHORT; }
  static inline MPI_Datatype map(int) { return MPI_INT; }
  static inline MPI_Datatype map(unsigned) { return MPI_UNSIGNED; }
  static inline MPI_Datatype map(long) { return MPI_LONG; }
  static inline MPI_Datatype map(unsigned long) { return MPI_UNSIGNED_LONG; }
  static inline MPI_Datatype map(long long) { return MPI_LONG_LONG; }
  static inline MPI_Datatype map(unsigned long long) {
    return MPI_UNSIGNED_LONG_LONG;
  }
  static inline MPI_Datatype map(float) { return MPI_FLOAT; }
  static inline MPI_Datatype map(double) { return MPI_DOUBLE; }
  static inline MPI_Datatype map(long double) { return MPI_LONG_DOUBLE; }

 public:  // Send
  /// Send a scalar value to another process
  template <typename Type>
  void send(const Type& value, const int toProcess, const int tag = 0) {
    return this->send(&value, 1, toProcess, tag, "value");
  }

  /// Send an array of count values to another process
  template <typename Type>
  void send(const Type* values, const int count, const int toProcess
      , const int tag = 0) {
    return this->send(values, count, toProcess, tag, "array");
  }

  /// Send an array of count values to another process (mutable)
  template <typename Type>
  void send(Type* values, const int count, const int toProcess
      , const int tag = 0) {
    return this->send(values, count, toProcess, tag, "array");
  }

  /// Send an array of values to another process
  template <typename Type>
  void send(const std::vector<Type>& values, const int toProcess
      , const int tag = 0) {
    // send vector's internal array instead of vector's address
    return this->send(values.data(), values.size(), toProcess, tag, "vector");
  }

  /// Send a text to another process
  void send(const std::string& text, const int toProcess, const int tag = 0) {
    // send text's array of chars
    return this->send(text.data(), text.size() + 1, toProcess, tag, "text");
  }


 public:  // Receive
  /// Wait until it receives a scalar value from other process
  template <typename Type>
  void receive(Type& value, const int fromProcess = MPI_ANY_SOURCE
      , const int tag = MPI_ANY_TAG) {
    return this->receive(&value, /*capacity*/ 1, fromProcess, tag, "value");
  }

  /// Wait until it receives at most capacity values from another process
  template <typename Type>
  void receive(Type* values, const int capacity
      , const int fromProcess = MPI_ANY_SOURCE, const int tag 
      = MPI_ANY_TAG ) {
    return this->receive(values, capacity, fromProcess, tag, "array");

  }

  /// Wait until it receives at most capacity values from another process
  template <typename Type>
  void receive(std::vector<Type>& values, const int capacity
      , const int fromProcess = MPI_ANY_SOURCE, const int tag = MPI_ANY_TAG) {
    // Match vector's capacity if necessary
    if (static_cast<int>(values.size()) < capacity) values.resize(capacity);
    return this->receive(values.data(), capacity, fromProcess, tag, "vector");
  }

  /// Wait until it receives a text of at most length chars from another process
  void receive(std::string& text, const int capacity
      , const int fromProcess = MPI_ANY_SOURCE, const int tag = MPI_ANY_TAG) {
    std::vector<char> buffer(capacity, '\0');
    this->receive(buffer, capacity, fromProcess, tag);
    text = buffer.data();  // Copy vector's data into text
  }

 private:
  /// Send an array of count values to another process
  template <typename Type>
  void send(const Type* values, const int count, const int toProcess
      , const int tag, const std::string& type) {
    // Verify if array has at least s1 value
    if (count <= 0) {
      throw Error("invalid count of elements to send");
    }

    // int MPI_Send(const void* buff, int count, MPI_Datatype datatype
        // , int dest, int tag, MPI_Comm comm)
    if (MPI_Send(values, count, Mpi::map(values[0]), toProcess
          , tag, MPI_COMM_WORLD) != MPI_SUCCESS) {
      throw Error("could not send " + type);
    }
  }

 private:  // RECEIVE PRIVATE
  template <typename Type>
  void receive(Type* values, const int capacity, const int fromProcess
      , const int tag, const std::string& type) {
    // Verify if array has at least s1 value
    if (capacity <= 0) {
      throw Error("invalid capacity of elements to receive");
    }

    // int MPI_Recv(void* buff, int capacity, MPI_Datatype datatype
        // , int source, int tag, MPI_Comm comm, MPI_Status status)
    if (MPI_Recv(values, capacity, Mpi::map(Type())
        , /*source*/ fromProcess, /*tag*/ tag, MPI_COMM_WORLD
        , MPI_STATUS_IGNORE) != MPI_SUCCESS) {
      throw Error("could not receive " + type);
    }
  }

 public:
  void barrier() {
    if (MPI_Barrier(MPI_COMM_WORLD) != MPI_SUCCESS) {
      throw Error("could not wait barrier");
    }
  }

  inline static double wtime() {
    return MPI_Wtime();
  }

 public:
  template <typename Type>
  void broadcast(Type& value, const int fromProcess) {
    // int MPI_Bcast(void *buffer, int count, MPI_DataType datatype, int root,
        // MPI_Comm comm);
    if (MPI_Bcast(&value, /*count*/ 1, Mpi::map(value), /*root*/ fromProcess,
          MPI_COMM_WORLD) != MPI_SUCCESS) {
      throw Error("could not broadcast");
    }
  }

 public:
  template <typename Type>
  void reduce(const Type& value, Type& result, const int operation,
      const int toProcess) {
    // int MPI_Reduce(const void *sendbuf, void *recvbuf, int count,
    //   MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
    if (MPI_Reduce(&value, &result, /*count*/ 1,
      Mpi::map(value), operation, toProcess, MPI_COMM_WORLD) != MPI_SUCCESS) {
      throw Error("could not reduce");
    }
  }
};
