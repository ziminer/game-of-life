#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include <vector>
#include "cell.h"

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
  bool Contains(unsigned long x,
                unsigned long y) const;

  // All points along the edge count.
  bool ContainsGreedy(unsigned long x,
                      unsigned long y) const;

  void Print() const;

  bool Intersects(const BoundingBox& box) const;
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

  void Divide();

public:
  QuadTree(const BoundingBox& boundary)
    : _boundary(boundary), _upperLeft(NULL), _upperRight(NULL),
      _lowerLeft(NULL), _lowerRight(NULL) {}

  ~QuadTree();

  void Clear();

  bool Insert(const Cell& point);

  void FindPoints(const BoundingBox& bound,
                  CellSet& out) const;

  // Debugging method
  void Print();

};

#endif
