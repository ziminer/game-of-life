#include "quadtree.h"
#include <cassert>
#include <iostream>

using namespace std;

void BoundingBox::Print() const {
  cout << "x: " << _x << " y: " << _y << " width: " << _width << " height: " << _height << endl;
}

bool BoundingBox::ContainsInside(unsigned long x,
                                 unsigned long y) const {
  return x > _x && x < (_x + _width) &&
         y > _y && y < (_y + _height);
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
  return Contains(box._x, box._y) ||
         Contains(box._x, box._y + box._height) ||
         Contains(box._x + box._width, box._y) ||
         Contains(box._x + box._width, box._y + box._height) ||
         box.ContainsInside(_x, _y) ||
         box.ContainsInside(_x, _y + _height) ||
         box.ContainsInside(_x + _width, _y) ||
         box.ContainsInside(_x + _width, _y + _height);
}

bool BoundingBox::IntersectsGreedy(const BoundingBox& box) const {
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
  if (_upperLeft != NULL) {
    delete _upperLeft;
  }
  if (_upperRight != NULL) {
    delete _upperRight;
  }
  if (_lowerLeft != NULL) {
    delete _lowerLeft;
  }
  if (_lowerRight != NULL) {
    delete _lowerRight;
  }
}

void QuadTree::Divide() {
  // Should only be called once
  if (_upperLeft != NULL) {
    return;
  }

  assert(!_points.empty());

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

  pair<unsigned long, unsigned long> point = *_points.begin();
  _points.pop_back();
  Insert(point);
  assert(_points.empty());
}

void QuadTree::Print() {
  if (_upperLeft != NULL) {
    assert(_points.empty());
    _upperLeft->Print();
    _upperRight->Print();
    _lowerLeft->Print();
    _lowerRight->Print();
  } else {
    for (vector< pair<unsigned long, unsigned long> >::const_iterator it = _points.begin();
         it != _points.end(); ++it) {
      cout << "x: " << it->first << " y: " << it->second << endl;
    }
  }
}

bool QuadTree::Insert(const pair<unsigned long, unsigned long>& point) {
  if (_boundary.Contains(point.first, point.second)) {
    if (_points.empty() && _upperLeft == NULL) {
      _points.push_back(point);
      return true;
    } else {
      Divide();
      assert(_points.empty());
      if (_upperLeft->Insert(point)) {
        return true;
      } else if (_upperRight->Insert(point)) {
        return true;
      } else if (_lowerLeft->Insert(point)) {
        return true;
      } else if (_lowerRight->Insert(point)) {
        return true;
      } else {
        return false;
      }
    }
  } else {
    return false;
  }
}

void QuadTree::QueryRange(const BoundingBox& bound,
                          set< pair<unsigned long, unsigned long> >& out) const {
  if (_boundary.IntersectsGreedy(bound)) {
    if (_upperLeft == NULL) {
      // This is a leaf node. Check it!
      for (vector< pair<unsigned long, unsigned long> >::const_iterator it = _points.begin();
           it != _points.end(); ++it) {
        if (bound.ContainsGreedy(it->first, it->second)) {
          out.insert(*it);
        }
      }
    } else {
      // This is a parent node.
      assert(_points.empty());
      _upperLeft->QueryRange(bound, out);
      _upperRight->QueryRange(bound, out);
      _lowerLeft->QueryRange(bound, out);
      _lowerRight->QueryRange(bound, out);
    }
  }
}
