#pragma once

#include <vector>

namespace datarotators {

/// \brief Container for storing multiple time steps of the simulation
template <class T>
class DataContainer {
  std::vector<T> container;

  public:

  size_t current_step = 0;

  void push_back(T vm);

  T* get();

  T* get_new();

  T* get_all(size_t cs);

  // FIXME raw cycling for time step index
  void cycle();

};


/// method to add data into the container
template <class T>
void datarotators::DataContainer<T>::push_back(T vm) {
  container.push_back(vm);
}


/// Get current element
template <class T>
T* datarotators::DataContainer<T>::get() {
  // fmt::print("getting from DataContainer with {}\n", current_step);
  return (T*) &(container[ current_step ]);
}

/// get a fresh container that we can update into
template <class T>
T* datarotators::DataContainer<T>::get_new() {
  if (current_step == 0) return (T*) &(container[1]);
  if (current_step == 1) return (T*) &(container[0]);
}


/// Get any arbitrary snapshot from the container
template <class T>
T* datarotators::DataContainer<T>::get_all(size_t cs) {
  // fmt::print("pulling from DataContainer with {}\n", cs);
  return (T*) &(container[cs]);
}


/// raw cycling for time step index
// NOTE: could be done better
template <class T>
void datarotators::DataContainer<T>::cycle() {
  // fmt::print(" calling cycle (originally {})\n", current_step);
  current_step++;

  // check bounds and cycle back
  if (current_step > 1) current_step = 0;
}



} // end of namespace datarotators
