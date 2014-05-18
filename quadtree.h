#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include <utility>
#include <vector>
#include <set>

class BoundingBox {
private:
  // Like Contains, but ignores border matches
  bool ContainsInside(unsigned long x,
                      unsigned long y) const;

public:

  // Left hand coord
  unsigned long _x;

  // Top coord
  unsigned long _y;

  unsigned long _width;

  unsigned long _height;

  BoundingBox(unsigned long x,
              unsigned long y,
              unsigned long width,
              unsigned long height)
    : _x(x), _y(y), _width(width), _height(height) {}

  bool Contains(unsigned long x,
                unsigned long y) const;

  bool ContainsGreedy(unsigned long x,
                      unsigned long y) const;

  void Print() const;

  bool Intersects(const BoundingBox& box) const;

  // Like intersects, but grabs everything at the borders
  bool IntersectsGreedy(const BoundingBox& box) const;

};

class QuadTree {
private:
  std::vector< std::pair<unsigned long, unsigned long> > _points;

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

  void Print();

  bool Insert(const std::pair<unsigned long, unsigned long>& point);

  void QueryRange(const BoundingBox& bound,
                  std::set< std::pair<unsigned long, unsigned long> >& out) const;
};

#endif
