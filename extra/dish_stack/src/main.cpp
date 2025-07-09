// Copyright 2025 ECCI-UCR CC-BY 4.0
#include <iostream>
#include <mutex>
#include <stack>
#include <thread>
#include <vector>

#define STACK_EMPTY_FLAG -1

// Thread-safe dish stack
class DishStack {
 private:
  std::stack<int> stack;
  std::mutex mutex;

 public:
  /// Return true if the stack is empty, false otherwise
  bool empty() {
    this->mutex.lock();
    const bool result = this->stack.empty();
    this->mutex.unlock();
    return result;
  }
  /// Return the number of dishes in the stack
  size_t size() {
    this->mutex.lock();
    const size_t result = this->stack.size();
    this->mutex.unlock();
    return result;
  }
  /// Push a value onto the stack, and it becomes the top element
  void push(const int& value) {
    this->mutex.lock();
    this->stack.push(value);
    this->mutex.unlock();
  }
  /// Pop the top element from the stack and return its value
  /// Precondition: the stack must be not empty, if it is, returns flag value -1
  int pop() {
    int value = STACK_EMPTY_FLAG;
    this->mutex.lock();
    // Pop value if stack is not empty
    if (!stack.empty()) {
      value = stack.top();
      stack.pop();
    }
    this->mutex.unlock();
    return value;
  }
};

DishStack dishes;

void wash(const int dish) {
  // No need to do anything in this version
  (void)dish;
}

void wash_dishes() {
  // Selected approach to fix atomicity violation was calling empty() inside pop
  // and use flag value to determine if it's done washing
  while (true) {
    // Pop a dish from the stack
    const int dish = dishes.pop();
    if (dish == STACK_EMPTY_FLAG) break;
    else wash(dish);
  }
}

int main() {
  // Fill the stack with dishes
  int washer_count = 0, dish_count = 0;
  std::cin >> washer_count >> dish_count;
  for (int counter = 0; counter < dish_count; ++counter) {
    dishes.push(counter);
  }
  // Hire dishwashers
  std::vector<std::thread> dishwashers;
  for (int counter = 0; counter < washer_count; ++counter) {
    dishwashers.emplace_back(std::thread(wash_dishes));
  }
  // Wait for all dishwashers to finish
  for (std::thread& dishwasher : dishwashers) {
    dishwasher.join();
  }
  // Be sure all dishes were washed
  std::cout << "Unwashed dishes: " << dishes.size() << std::endl;
  return 0;
}
