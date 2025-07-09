// Copyright 2025 ECCI-UCR CC-BY 4.0
#include <semaphore>
#include <barrier>
#include <cassert>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

size_t molecule_count = 0;  // Number of molecules formed
std::barrier bond_barrier(3);  // Control enter and exit of 3 atoms in bond area
std::counting_semaphore hydrogen_multiplex(2);  // 2 hydrogen atoms are allowed
std::counting_semaphore oxygen_multiplex(1);  // 1 oxygen atom is needed for H2O

void bond(const char atom);
void hydrogen();
void oxygen();

int main() {
  // Create threads that represent atoms according to values in stdin
  char atom_type{};
  std::vector<std::thread> threads;
  while (std::cin >> atom_type) {
    switch (atom_type) {
      case 'H': threads.emplace_back(std::thread(hydrogen)); break;
      case 'O': threads.emplace_back(std::thread(oxygen)); break;
      default: assert(false); break;
    }
  }
  // Wait for all atoms to finish
  for (std::thread& thread : threads) {
    thread.join();
  }
  // Report number of molecules formed
  std::cout << "Molecules formed: " << molecule_count << std::endl;
  return 0;
}

void hydrogen() {
  // Wait for hydrogen's semaphore
  hydrogen_multiplex.acquire();
  // Wait for other atoms to arrive
  bond_barrier.arrive_and_wait();
  bond('H');
  // Exit in unison after bonding
  bond_barrier.arrive_and_wait();
  // Make way for the next batch of hydrogen atoms
  hydrogen_multiplex.release();
}

void oxygen() {
  // See hydrogen() comments for logic
  oxygen_multiplex.acquire();
  bond_barrier.arrive_and_wait();
  bond('O');
  bond_barrier.arrive_and_wait();
  oxygen_multiplex.release();
}

// Do not modify this procedure:
void bond(const char atom) {
  // Check two hydrogen and one oxygen atoms are in the device to form water
  static size_t hydrogen_count = 0, oxygen_count = 0;
  static std::mutex mutex;
  mutex.lock();
  switch (atom) {
    case 'H': ++hydrogen_count; break;
    case 'O': ++oxygen_count; break;
    default: assert(false); break;
  }
  if (hydrogen_count == 2 && oxygen_count == 1) {
    ++molecule_count;
    hydrogen_count = oxygen_count = 0;  // Reset counts for next molecule
    mutex.unlock();
    // Here the code to control the device will be added in the future
    // device.induce();
  } else {
    mutex.unlock();
  }
}
