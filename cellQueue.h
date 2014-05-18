#ifndef __CELL_QUEUE_H__
#define __CELL_QUEUE_H__

#include "game.h"
#include <queue>

/**
 * Utility class - queue that rejects duplicates.
 *
 * We can only insert an element into this queue once.
 * Cannot insert an equal element even if we pop the
 * original one.
 *
 * TODO: (low priority) make template
 */
class CellQueue {
private:
  std::queue<Cell> _queue;

  CellSet _elements;

public:
  CellQueue() {}

  CellQueue(const CellSet& cells)
    : _elements(cells) {
    for (CellSet::const_iterator it = cells.begin();
         it != cells.end(); ++it) {
      _queue.push(*it);
    }
  }

  bool Push(const Cell& cell);

  Cell& Front();

  bool Empty() const;

  void Pop();
};

#endif
