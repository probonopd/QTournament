#include "ThreadSafeQueue.h"


ProgressQueue::ProgressQueue(int _maxVal)
{
  if (_maxVal <= 0)
  {
    throw std::invalid_argument("Can't have a max progress alue less or equal to zero");
  }

  maxVal = _maxVal;

  scaleFac = 100.0 / maxVal;
}

void ProgressQueue::step(int numSteps)
{
  if (numSteps <= 0) return;

  // acquire a lock on the mutex
  lock_guard<mutex> lk{queueMutex};

  // calculate the new value
  counter += numSteps;
  if (counter > maxVal) counter = maxVal;

  // map the new value to a 0...100 range and store the it
  q.push((int)(counter * scaleFac));

  // notify potentially waiting threads that
  // the queue is not empty anymore
  emptyCondition.notify_one();

  // lk is automatically unlocked by the
  // destructor of the lock_guard
}

void ProgressQueue::reset(int _maxVal)
{
  if (_maxVal <= 0)
  {
    throw std::invalid_argument("Can't have a max progress alue less or equal to zero");
  }

  // acquire a lock on the mutex
  lock_guard<mutex> lk{queueMutex};

  // clear all old items from the list and
  // reinitialize members
  queue<int> empty;
  q.swap(empty);
  maxVal = _maxVal;
  scaleFac = 100.0 / maxVal;
  counter = 0;
}
