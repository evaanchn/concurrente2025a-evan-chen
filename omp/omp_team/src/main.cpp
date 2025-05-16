// Copyright 2024 ECCI-UCR CC-BY 4.0
#include <iostream>
#include <omp.h>

void heavy_task() {
}

int main(int argc, char* argv[]) {
  // If no thead count specified, use omp max amount of threads (core count)
  const int thread_count = argc == 2 ? ::atoi(argv[1]) :
      omp_get_max_threads();

  // Open MP is declarative: we don't have as much control but it's easy
  // PARALLEL IMPLIES THREAD TEAM CREATION
  // numm_threads clause sets thread count as amount of threads used
  // Everything is shared by default, so we disactivate with default(none)
  // std::cout must be shared, so that cursor is not distributed to diff threads
  // private() does not initialize, does not really work
  // use firstprivate() instead, which assigns the value of original variable
  #pragma omp parallel num_threads(thread_count) \
      default(none) shared(std::cout) firstprivate(thread_count)
  {
    heavy_task();  // Hypothetical subroutine
    // Prints this thread_count of times in parallel

    // At processor level, it generates a routine and expands into pthread code

    // Parallel block with critical, indicating that it's a critical region
    // This is the equivalent of adding a mutex, named stdout to differentiate
    #pragma omp critical(stdout)
    std::cout << "Hello from secondary thread " << omp_get_thread_num()
      << " of " << thread_count << std::endl;
  }
  // Implicit join after execution. Main thread continues after this
  // Thus the following print will always occur after secondary greets
  std::cout << "Hello from main thread " << std::endl;
}
