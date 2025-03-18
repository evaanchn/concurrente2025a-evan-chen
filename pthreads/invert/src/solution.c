// Copyright <year> <You>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Start program execution.
 *
 * @return Status code to the operating system, 0 means success.
 */
int main(void) {
  // Create a dynamic array to store the numbers, given we don't know the number of elements beforehand
  size_t count = 0, capacity = 1000;
  long* numbers = malloc(capacity * sizeof(long));

  // Read numbers until end-of-file
  while (scanf("%ld", &numbers[count]) ==  1) {
    // If capacity is reached
    if (++count == capacity) {
      // expand capacity of array ten-fold
      capacity *= 10;
      numbers = realloc(numbers, capacity * sizeof(long));
    }
  }

  // Print all numbers in inverse order
  for (long index = count - 1; index >= 0; --index) {
    printf("%ld\n", numbers[index]);
  }

  // Deallocate dynamic array
  free(numbers);
  return EXIT_SUCCESS;
}
