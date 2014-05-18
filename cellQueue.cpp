#include "cellQueue.h"
#include "game.h"
#include <iostream>

using namespace std;

bool CellQueue::Push(const Cell& cell) {
  if (_elements.insert(cell).second) {
    _queue.push(cell);
    return true;
  } else {
    return false;
  }
}

Cell& CellQueue::Front() {
  return _queue.front();
}

bool CellQueue::Empty() const {
  return _queue.empty();
}

void CellQueue::Pop() {
  _queue.pop();
}
