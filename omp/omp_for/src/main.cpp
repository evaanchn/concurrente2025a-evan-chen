// Copyright 2024 ECCI-UCR CC-BY 4.0
#include <iostream>
#include <omp.h>

int main(int argc, char* argv[]) {
  const int thread_count = argc >= 2 ? ::atoi(argv[1])
      : omp_get_max_threads();
  const int iteration_count = argc >= 3 ? ::atoi(argv[2]) : thread_count;

  // parallel for distributes the iterations to do. CANNOT ADD A BREAK INSIDE
  // MAP with schedule()
  // schedule(static) or schedule(auto) static by blocks
  // schedule(static, num), Cyclic, can be by blocks if num > 1
  // schedule(dynamic, num), dynamic mapping, num determines block size
  // schedule(guided, num), At the start, assigns big blocks
  //   , but blocks reduce in size when approaching end
  //   , where num is the ending blocks size
  // schedule(runtime), map can be defined at execution time
  //   , with OMP_SCHEDULE = [map].
  //   This also implies that it implements ALL maps
  #pragma omp parallel for num_threads(thread_count) schedule(runtime) \
      default(none) shared(std::cout, iteration_count, thread_count)
  for (int iteration = 0; iteration < iteration_count; ++iteration) {
    #pragma omp critical(stdout)
    std::cout << omp_get_thread_num() << '/' << thread_count
      << ": iteration " << iteration << '/' << iteration_count << std::endl;
  }
}
