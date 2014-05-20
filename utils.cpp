#include <cassert>
#include <iostream>
#include <queue>

#include "utils.h"

using namespace std;


unsigned long
ApplyOffset(unsigned long target,
            unsigned long offset,
            bool neg) {
  if (neg) {
    if (offset > target) {
      throw out_of_range("Offset causes overflow");
    }
    return target - offset;
  } else {
    if (target > (ULONG_MAX - offset)) {
      throw out_of_range("Offset causes overflow");
    }
    return target + offset;
  }
}


unsigned long
CalculateOffset(unsigned long from,
                unsigned long to,
                bool *neg) {
  if (from > to) {
      *neg = true;
      return from - to;
  } else {
      *neg = false;
      return to - from;
  }
}


bool
BoundingBox::Contains(unsigned long x,
                      unsigned long y) const {
  return x > _x && x <= (_x + _width) &&
         y >= _y && y < (_y + _height);
}


bool
BoundingBox::ContainsGreedy(unsigned long x,
                            unsigned long y) const {
  return x >= _x && x <= (_x + _width) &&
         y >= _y && y <= (_y + _height);
}


bool
BoundingBox::Intersects(const BoundingBox& box) const {
  return !((box._x < (ULONG_MAX - box._width) &&
            _x > (box._x + box._width)) ||
           (_x < (ULONG_MAX - _width) &&
           (_x + _width) < box._x) ||
           (box._y < (ULONG_MAX - box._height) &&
            _y > (box._y + box._height)) ||
           (_y < (ULONG_MAX - _height) &&
           (_y + _height) < box._y));
}


QuadTree::~QuadTree() {
  Clear();
}


void
QuadTree::Clear() {
  _cells.clear();
  if (_upperLeft != NULL) {
    delete _upperLeft;
    _upperLeft = NULL;
  }
  if (_upperRight != NULL) {
    delete _upperRight;
    _upperRight = NULL;
  }
  if (_lowerLeft != NULL) {
    delete _lowerLeft;
    _lowerLeft = NULL;
  }
  if (_lowerRight != NULL) {
    delete _lowerRight;
    _lowerRight = NULL;
  }
  assert(_cells.empty());
}


bool
CellQueue::Push(const Cell& cell) {
  if (_elements.insert(cell).second) {
    _queue.push(cell);
    return true;
  } else {
    return false;
  }
}


Cell&
CellQueue::Front() {
  return _queue.front();
}


bool
CellQueue::Empty() const {
  return _queue.empty();
}


void
CellQueue::Pop() {
  _queue.pop();
}


void
QuadTree::Divide() {
  // Should only be called once
  if (_upperLeft != NULL) {
    return;
  }

  assert(!_cells.empty());

  unsigned long leftWidth = _boundary._width / 2;
  unsigned long rightWidth = leftWidth + _boundary._width % 2;
  unsigned long topHeight = _boundary._height / 2;
  unsigned long bottomHeight = topHeight + _boundary._height %2;

  _upperLeft = new QuadTree(BoundingBox(_boundary._x, _boundary._y,
                                       leftWidth, topHeight));
  _upperRight = new QuadTree(BoundingBox(_boundary._x + leftWidth,
                                        _boundary._y, rightWidth, topHeight));
  _lowerLeft = new QuadTree(BoundingBox(_boundary._x, _boundary._y + topHeight,
                                       leftWidth, bottomHeight));
  _lowerRight = new QuadTree(BoundingBox(_boundary._x + leftWidth,
                                        _boundary._y + topHeight,
                                        rightWidth, bottomHeight));

  Cell cell = *_cells.begin();
  _cells.pop_back();
  Insert(cell);
  assert(_cells.empty());
}


bool
QuadTree::Insert(const Cell& cell) {
  if (_boundary.Contains(cell.x, cell.y)) {
    if (_cells.empty() && _upperLeft == NULL) {
      _cells.push_back(cell);
      return true;
    } else {
      Divide();
      assert(_cells.empty());
      if (_upperLeft->Insert(cell)) {
        return true;
      } else if (_upperRight->Insert(cell)) {
        return true;
      } else if (_lowerLeft->Insert(cell)) {
        return true;
      } else if (_lowerRight->Insert(cell)) {
        return true;
      } else {
        return false;
      }
    }
  } else {
    return false;
  }
}


void
QuadTree::FindPoints(const BoundingBox& bound,
                     CellSet& out) const {
  if (_boundary.Intersects(bound)) {
    if (_upperLeft == NULL) {
      // This is a leaf node. Check it!
      for (vector<Cell>::const_iterator it = _cells.begin();
           it != _cells.end(); ++it) {
        if (bound.ContainsGreedy(it->x, it->y)) {
          out.insert(*it);
        }
      }
    } else {
      // This is a parent node.
      assert(_cells.empty());
      _upperLeft->FindPoints(bound, out);
      _upperRight->FindPoints(bound, out);
      _lowerLeft->FindPoints(bound, out);
      _lowerRight->FindPoints(bound, out);
    }
  }
}

