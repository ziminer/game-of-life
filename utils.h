#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include <vector>
#include <queue>

#include "cell.h"


/*
 * Throws out_of_range if adding/subtracting offset causes overflow.
 */

unsigned long
ApplyOffset(unsigned long target,
            unsigned long offset,
            bool neg);


unsigned long
CalculateOffset(unsigned long from,
                unsigned long to,
                bool *neg);


/**
 * Rectangle for collision detection
 */

class BoundingBox {
public:

  // Left hand coord
  unsigned long _x;

  // Top coord
  unsigned long _y;

  unsigned long _width;

  unsigned long _height;

  BoundingBox()
    : _x(0), _y(0), _width(0), _height(0) {}

  BoundingBox(unsigned long x,
              unsigned long y,
              unsigned long width,
              unsigned long height)
    : _x(x), _y(y), _width(width), _height(height) {}

  /*
   * Only points along top and right edges count
   * This is so that each point is in only one
   * leaf node of the quadtree.
   */
  bool
  Contains(unsigned long x,
           unsigned long y) const;

  // All points along the edge count.
  bool
  ContainsGreedy(unsigned long x,
                 unsigned long y) const;

  bool
  Intersects(const BoundingBox& box) const;
};


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

  bool
  Push(const Cell& cell);

  Cell&
  Front();

  bool
  Empty() const;

  void
  Pop();
};


/*
 * Simple region quadtree implementation, with the limit
 * per region at 1. Stores Cell objects.
 *
 * TODO: (not important) make template
 *
 * See: http://en.wikipedia.org/wiki/Quadtree
 */

class QuadTree {
private:
  std::vector<Cell> _cells;

  BoundingBox _boundary;

  QuadTree *_upperLeft;

  QuadTree *_upperRight;

  QuadTree *_lowerLeft;

  QuadTree *_lowerRight;

  void
  Divide();

public:
  QuadTree(const BoundingBox& boundary)
    : _boundary(boundary), _upperLeft(NULL), _upperRight(NULL),
      _lowerLeft(NULL), _lowerRight(NULL) {}

  ~QuadTree();

  void
  Clear();

  bool
  Insert(const Cell& point);

  void
  FindPoints(const BoundingBox& bound,
             CellSet& out) const;

};

#endif
