#include <set>
#include <utility>
#include "game.h"
#include "quadtree.h"
#include <iostream>
#include <cassert>

using namespace std;

void testBoundingBox() {
  BoundingBox box(10, 20, 20, 20);
  cout << "Testing bounding box Contains..." << endl;
  // Top left
  assert(!box.Contains(10, 20));
  // Bottom right
  assert(!box.Contains(10, 40));
  // Bottom right
  assert(!box.Contains(30, 40));
  // Top right
  assert(box.Contains(30, 20));
  // Top border
  assert(box.Contains(13, 20));
  // Right border
  assert(box.Contains(30, 30));
  // Negative
  assert(!box.Contains(0,0));
  // Positive
  assert(box.Contains(20, 30));
  cout << "Contains tests passed..." << endl;

  cout << "Testing bounding box Intersects..." << endl;
  // Completely overlap, smaller
  assert(box.Intersects(BoundingBox(15, 25, 10, 10)));
  // Completely overlap, bigger
  assert(box.Intersects(BoundingBox(5, 10, 100, 100)));
  // Right side X
  assert(box.Intersects(BoundingBox(5, 20, 20, 20)));
  // Right side top
  assert(box.Intersects(BoundingBox(5, 15, 20, 20)));
  // Right side bottom
  assert(box.Intersects(BoundingBox(5, 35, 20, 20)));
  // Left side X
  assert(box.Intersects(BoundingBox(25, 20, 20, 20)));
  // Left side top
  assert(box.Intersects(BoundingBox(25, 15, 20, 20)));
  // Left side bottom
  assert(box.Intersects(BoundingBox(25, 35, 20, 20)));
  // Top
  assert(box.Intersects(BoundingBox(10, 5, 20, 20)));
  // Bottom
  assert(box.Intersects(BoundingBox(10, 35, 20, 20)));
  // Negative
  assert(!box.Intersects(BoundingBox(9, 11, 2, 2)));
  // Adjacent
  assert(!box.Intersects(BoundingBox(0, 20, 10, 10)));
  assert(!box.Intersects(BoundingBox(40, 20, 20, 20)));
  // Top edge should work
  assert(box.Intersects(BoundingBox(10, 0, 20, 20)));
  assert(!box.Intersects(BoundingBox(20, 40, 20, 20)));
  // Right edge should work
  assert(box.Intersects(BoundingBox(30, 20, 20, 20)));

  // Random tests
  BoundingBox box1(0, 1, 1, 2);
  assert(box1.Intersects(BoundingBox(0, 0, 2, 2)));
  cout << "Intersects tests passed" << endl;
}

void testQuadTree() {
  QuadTree tree(BoundingBox(0, 0, LONG_MAX, LONG_MAX));
  cout << "Testing quad tree..." << endl;
  tree.Insert(pair<unsigned long, unsigned long>(1,1));
  tree.Insert(pair<unsigned long, unsigned long>(2,2));
  tree.Insert(pair<unsigned long, unsigned long>(1,0));
  tree.Insert(pair<unsigned long, unsigned long>(1,2));
  tree.Print();
  set< pair<unsigned long, unsigned long> > results;
  tree.QueryRange(BoundingBox(0, 0, 10, 10), results);
  assert(results.size() == 4);
  results.clear();

  tree.QueryRange(BoundingBox(0, 0, 1, 1), results);
  assert(results.size() == 2);
  results.clear();

  tree.QueryRange(BoundingBox(0, 0, 2, 2), results);
  cout << results.size() << endl;
  assert(results.size() == 4);
  results.clear();

  tree.QueryRange(BoundingBox(0, 0, 2, 2), results);
  assert(results.size() == 4);
  results.clear();

  tree.QueryRange(BoundingBox(1, 1, 2, 2), results);
  assert(results.size() == 3);
  results.clear();

  tree.QueryRange(BoundingBox(0, 0, 2, 1), results);
  assert(results.size() == 2);
  results.clear();

  cout << "Quad tree tests passed" << endl;
}

int main() {
  testBoundingBox();
  testQuadTree();
  set<Cell> starterSet;
  cout << starterSet.insert(Cell(1,1)).second << endl;
  cout << starterSet.insert(Cell(1,2)).second << endl;
  cout << starterSet.insert(Cell(1,1)).second << endl;
  cout << starterSet.insert(Cell(2,1)).second << endl;
  cout << starterSet.insert(Cell(2,1)).second << endl;
  Game game(starterSet);
  game.Start();

  return 0;
}
