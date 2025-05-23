// Copyright 2023 Jeisson Hidalgo jeisson.hidalgo@ucr.ac.cr CC-BY-4
#include <omp.h>
#include <algorithm>
#include <vector>

// size_t y ptrdiff_t adapt to architecture, the latter permits negatives
template <typename Type>
void mergesort(std::vector<Type>& values, const ptrdiff_t left,
    const ptrdiff_t right) {
  // Count of elements we have to sort
  const ptrdiff_t count = right - left;
  if (count > 1024) {
    const size_t mid = (left + right) / 2;
    #pragma omp taskgroup
    {
      // Sort left half of the array
      // Enqueues task for posterior execution
      // The if makes sure if it's worth the hassle
      #pragma omp task untied if (mid - left >= 5000) \
          default(none) shared(values) firstprivate(left, mid)  
      mergesort(values, left, mid);
      // Sort right half of the array
      #pragma omp task untied default(none) shared(values) \
          firstprivate(mid, right)
      mergesort(values, mid + 1, right);
      // Wait until both sub-vectors are sorted
      #pragma omp taskwait
      // #pragma omp taskyield  // dont't stay waiting, keep working
    }

    
    // Both halves are sorted, merge them into a temp vector
    std::vector<Type> temp;
    temp.reserve(count + 1);
    std::merge(values.begin() + left, values.begin() + mid + 1,
        values.begin() + mid + 1, values.begin() + right + 1,
        std::back_inserter(temp));
    // Copy the sorted temp vector back to the original vector
    std::copy(temp.begin(), temp.end(), values.begin() + left);
  } else {
    // Sorts the small section, since there were so little data that it
    // doesn't make sense to let threads do tiny work
    std::sort(values.begin() + left, values.begin() + right + 1);
  }
}

template <typename Type>
void mergesort(std::vector<Type>& values,
    int thread_count = omp_get_max_threads()) {
  #pragma omp parallel num_threads(thread_count) default(none) shared(values)
  // single means only one thread executes mergesort
  // The rest of the threads go to the end of the parallel section
  // In this case, it becomes a producer
  // Where consumes and joins lie
  #pragma omp single
  mergesort(values, 0, values.size() - 1);
}
