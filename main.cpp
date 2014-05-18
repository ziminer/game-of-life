#include <set>
#include <unordered_set>
#include <utility>
#include "game.h"
#include "cellQueue.h"
#include "quadtree.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <cstdlib>

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
  // Adjacent - all should succeed
  assert(box.Intersects(BoundingBox(0, 20, 10, 10)));
  assert(box.Intersects(BoundingBox(10, 0, 20, 20)));
  assert(box.Intersects(BoundingBox(20, 40, 20, 20)));
  assert(box.Intersects(BoundingBox(30, 20, 20, 20)));

  // Random tests
  BoundingBox box1(0, 1, 1, 2);
  assert(box1.Intersects(BoundingBox(0, 0, 2, 2)));
  cout << "Intersects tests passed" << endl;
}

void testQuadTree() {
  QuadTree tree(BoundingBox(0, 0, ULONG_MAX, ULONG_MAX));
  cout << "Testing quad tree..." << endl;
  tree.Insert(Posn(1,1));
  tree.Insert(Posn(2,2));
  tree.Insert(Posn(1,0));
  tree.Insert(Posn(1,2));
  set<Posn> results;
  tree.FindPoints(BoundingBox(0, 0, 10, 10), results);
  assert(results.size() == 4);
  results.clear();

  tree.FindPoints(BoundingBox(0, 0, 1, 1), results);
  assert(results.size() == 2);
  results.clear();

  tree.FindPoints(BoundingBox(0, 0, 2, 2), results);
  assert(results.size() == 4);
  results.clear();

  tree.FindPoints(BoundingBox(0, 0, 2, 2), results);
  assert(results.size() == 4);
  results.clear();

  tree.FindPoints(BoundingBox(1, 1, 2, 2), results);
  assert(results.size() == 3);
  results.clear();

  tree.FindPoints(BoundingBox(0, 0, 2, 1), results);
  assert(results.size() == 2);
  results.clear();

  cout << "Quad tree tests passed" << endl;
}

void testCellComps() {
  cout << "Cell comparison tests..." << endl;
  CellSet starterSet;
  assert(starterSet.insert(Cell(1,1)).second);
  assert(starterSet.insert(Cell(1,2)).second);
  assert(!starterSet.insert(Cell(1,1)).second);
  assert(starterSet.insert(Cell(2,1)).second);
  assert(!starterSet.insert(Cell(2,1)).second);
  assert(!starterSet.insert(Cell(2,1, false)).second);
  assert(starterSet.insert(Cell(1,0)).second);
  assert(!starterSet.insert(Cell(1,0)).second);
  assert(starterSet.insert(Cell(0,0)).second);
  assert(!starterSet.insert(Cell(1,2)).second);

  starterSet.clear();
  starterSet.insert(Cell(2,3));
  starterSet.insert(Cell(1,3));
  starterSet.insert(Cell(0,3));
  starterSet.insert(Cell(3,2));
  starterSet.insert(Cell(2,2));
  starterSet.insert(Cell(1,2));
  starterSet.insert(Cell(0,2));
  starterSet.insert(Cell(3,1));
  starterSet.insert(Cell(3,0));
  starterSet.insert(Cell(2,1));
  starterSet.insert(Cell(2,0));
  starterSet.insert(Cell(1,1));
  starterSet.insert(Cell(0,1));
  starterSet.insert(Cell(0,0));
  starterSet.insert(Cell(1,0));
  assert(!starterSet.insert(Cell(1,2, false)).second);
  assert(!starterSet.insert(Cell(1,0, false)).second);
  cout << "Cell comparisons passed" << endl;
}

void testCellQueue() {
  cout << "Cell queue tests.." <<endl;
  CellQueue cQueue;
  assert(cQueue.Push(Cell(1,2)));
  assert(cQueue.Front() == Cell(1,2));
  assert(!cQueue.Push(Cell(1,2)));
  assert(cQueue.Push(Cell(2, 1)));
  cQueue.Pop();
  // Can't put a cell that we already had in again
  assert(!cQueue.Push(Cell(1,2)));
  assert(!cQueue.Push(Cell(1,2,false)));
  assert(cQueue.Front() == Cell(2, 1));

  CellSet startSet;
  startSet.insert(Cell(1,2));
  startSet.insert(Cell(2,1));
  CellQueue cQueue2(startSet);
  cQueue2.Pop();
  cQueue2.Pop();
  assert(!cQueue2.Push(Cell(1,2)));
  assert(!cQueue2.Push(Cell(2,1)));
  cout << "Cell queue passed" << endl;
}

int main(int argc, char ** argv) {
  testBoundingBox();
  testQuadTree();
  testCellComps();

  CellSet starterSet;

  // Read input from file, or from default
  const char * fileName = "config.cfg";
  if (argc == 2) {
    fileName = argv[1];
  } else if (argc > 2) {
    cerr << "Usage: game-of-life [config file]" << endl;
    return 1;
  }

  ifstream configFile (fileName);
  string line;
  if (configFile.is_open()) {
    while (getline(configFile, line)) {
      size_t pos = line.find(" ");
      if (pos == string::npos) {
        cerr << "Invalid config line. Expecting space-separated longs" << endl;
        return 1;
      } else {
        string xStr = line.substr(0, pos);
        string yStr = line.substr(pos+1);
        long x = atol(xStr.c_str());
        // TODO: (not important) what if string is 0000
        if (x == 0 && xStr != "0") {
          cerr << "Could not convert " << xStr << " to long" << endl;
          return 1;
        }
        long y = atol(yStr.c_str());
        if (y == 0 && yStr != "0") {
          cerr << "Could not convert " << yStr << " to long" << endl;
          return 1;
        }
        // Input format in signed long, but want to deal with unsigned
        // long internally so convert here.
        unsigned long adjustedX = x + LONG_MAX + 1;
        unsigned long adjustedY = y + LONG_MAX + 1;
        starterSet.insert(Cell(adjustedX, adjustedY));
      }
    }
  }

  Game game(starterSet);
  game.Start();

  return 0;
}
