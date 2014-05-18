#include "quadtree.h"
#include <cassert>
#include <iostream>

using namespace std;

void BoundingBox::Print() const {
  cout << "x: " << _x << " y: " << _y << " width: " << _width << " height: " << _height << endl;
}

bool BoundingBox::Contains(unsigned long x,
                           unsigned long y) const {
  return x > _x && x <= (_x + _width) &&
         y >= _y && y < (_y + _height);
}

bool BoundingBox::ContainsGreedy(unsigned long x,
                           unsigned long y) const {
  return x >= _x && x <= (_x + _width) &&
         y >= _y && y <= (_y + _height);
}

bool BoundingBox::Intersects(const BoundingBox& box) const {
  return ContainsGreedy(box._x, box._y) ||
         ContainsGreedy(box._x, box._y + box._height) ||
         ContainsGreedy(box._x + box._width, box._y) ||
         ContainsGreedy(box._x + box._width, box._y + box._height) ||
         box.ContainsGreedy(_x, _y) ||
         box.ContainsGreedy(_x, _y + _height) ||
         box.ContainsGreedy(_x + _width, _y) ||
         box.ContainsGreedy(_x + _width, _y + _height);
}

QuadTree::~QuadTree() {
  Clear();
}

void QuadTree::Clear() {
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
}

void QuadTree::Divide() {
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

void QuadTree::Print() {
  if (_upperLeft != NULL) {
    assert(_cells.empty());
    _upperLeft->Print();
    _upperRight->Print();
    _lowerLeft->Print();
    _lowerRight->Print();
  } else {
    for (vector<Cell>::const_iterator it = _cells.begin();
         it != _cells.end(); ++it) {
      cout << "x: " << it->x << " y: " << it->y << endl;
    }
  }
}

bool QuadTree::Insert(const Cell& cell) {
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

void QuadTree::FindPoints(const BoundingBox& bound,
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
