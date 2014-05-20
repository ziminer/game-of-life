#include <queue>
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <fstream>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "gameBoard.h"
#include "utils.h"

using namespace std;

static const int INITIAL_CELL_SIZE = 20;

static const int CELL_SIZE_INCREMENT = 5;

static const int MAX_CELL_SIZE = 100;

static const int MIN_CELL_SIZE = 5;

static const sf::Color BACKGROUND_COLOUR = sf::Color(253, 246, 227);

static const sf::Color CELL_COLOUR = sf::Color(88,110,117);

static const sf::Color GRID_COLOUR = sf::Color(238, 232, 213);


void
Cell::Draw(const ViewInfo& view,
           sf::RenderTarget& texture,
           sf::Color colour) const {
  int xOffset = x - view.viewBox._x;
  int yOffset = y - view.viewBox._y;
  sf::CircleShape shape(view.cellSize/2);
  shape.setFillColor(colour);
  shape.setOutlineThickness(1);
  shape.setOutlineColor(BACKGROUND_COLOUR);
  shape.setPosition(xOffset * view.cellSize, yOffset * view.cellSize);
  texture.draw(shape);
}


void
ViewInfo::Move(MoveDirection direction) {
  try {
    switch (direction) {
      case MOVE_UP:
        yCentre = ApplyOffset(yCentre, 1, true);
        break;
      case MOVE_DOWN:
        yCentre = ApplyOffset(yCentre, 1, false);
        break;
      case MOVE_LEFT:
        xCentre = ApplyOffset(xCentre, 1, true);
        break;
      case MOVE_RIGHT:
        xCentre = ApplyOffset(xCentre, 1, false);
        break;
    }
    UpdateBox(xCentre, yCentre, screenWidth, screenHeight, cellSize);
  } catch (const out_of_range& err) {
    cerr << "Unable to move - out of range!" << endl;
  }
}


void
ViewInfo::Init(int width,
               int height,
               unsigned long x,
               unsigned long y) {
  UpdateBox(x, y, width, height, INITIAL_CELL_SIZE);
  xCentre = x;
  yCentre = y;
  cellSize = INITIAL_CELL_SIZE;
  screenWidth = width;
  screenHeight = height;
}


void
ViewInfo::Resize(int width,
                 int height) {
  try {
    UpdateBox(xCentre, yCentre, width, height, cellSize);
    screenWidth = width;
    screenHeight = height;
  } catch (const out_of_range& err) {
    cerr << "Resize failed due to overflow" << endl;
  }
}


void
ViewInfo::Centre(unsigned long x,
                 unsigned long y) {
  try {
    UpdateBox(x, y, screenWidth, screenHeight, cellSize);
    xCentre = x;
    yCentre = y;
  } catch (const out_of_range& err) {
    cerr << "Centre failed due to overflow" << endl;
  }
}


void
ViewInfo::UpdateBox(unsigned long newXCentre,
                    unsigned long newYCentre,
                    int newWidth,
                    int newHeight,
                    int newCellSize) {
  BoundingBox newViewBox;
  // Add 1 to width and height to get cells that
  // are only partially on-screen.
  newViewBox._width = newWidth / newCellSize + 1;
  newViewBox._height = newHeight / newCellSize + 1;
  newViewBox._x = ApplyOffset(newXCentre, newViewBox._width / 2, true);
  newViewBox._y = ApplyOffset(newYCentre, newViewBox._height / 2, true);
  // Swap at the end in case of overflow
  viewBox = newViewBox;
}


void
ViewInfo::Zoom(ZoomDirection direction) {
  int newCellSize;
  switch (direction) {
    case ZOOM_IN:
      newCellSize = min(cellSize + CELL_SIZE_INCREMENT, MAX_CELL_SIZE);
    break;
    case ZOOM_OUT:
      newCellSize = max(cellSize - CELL_SIZE_INCREMENT, MIN_CELL_SIZE);
    break;
  }
  try {
    UpdateBox(xCentre, yCentre, screenWidth, screenHeight, newCellSize);
    cellSize = newCellSize;
  } catch (const out_of_range& err) {
    cerr << "Zoom failed due to overflow" << endl;
  }
}


Cell
ViewInfo::PosnToCell(int x,
                     int y) {
  /*
   * What to do with the remainder after the division depends on
   * which quadrand we're in.
   */
  int xDiff = (x - (screenWidth / 2)) / cellSize;
  int xDiffRemainder = (x - (screenWidth / 2)) % cellSize;
  if (xDiff < 0 || (xDiff == 0 && xDiffRemainder < 0)) {
    xDiffRemainder = xDiffRemainder == 0 ? 0 : xDiffRemainder / xDiffRemainder;
    xDiff = (xDiffRemainder > 0) ? xDiff - xDiffRemainder : xDiff + xDiffRemainder;
  }
  int yDiff = (y - (screenHeight / 2)) / cellSize; 
  int yDiffRemainder = (y - (screenHeight / 2)) % cellSize;
  if (yDiff < 0 || (yDiff == 0 && yDiffRemainder < 0)) {
    yDiffRemainder = yDiffRemainder == 0 ? 0 : yDiffRemainder / yDiffRemainder;
    yDiff = (yDiffRemainder > 0) ? yDiff - yDiffRemainder : yDiff + yDiffRemainder;
  }
  
  return Cell(ApplyOffset(xCentre, abs(xDiff), xDiff < 0),
              ApplyOffset(yCentre, abs(yDiff), yDiff < 0));
}


GameBoard::GameBoard(const CellSet& points)
  : _initialCells(points), _liveCells(points),
    _quadTree(BoundingBox(0, 0, ULONG_MAX, ULONG_MAX)),
    _changeQuadTree(BoundingBox(0, 0, ULONG_MAX, ULONG_MAX)),
    _patternQuadTree(BoundingBox(0, 0, ULONG_MAX, ULONG_MAX))
{
  for (CellSet::const_iterator it = points.begin();
       it != points.end(); ++it) {
    _quadTree.Insert(*it);
  } 
}


Cell
GameBoard::FindNearest(const Cell& cell) const {
  if (_liveCells.count(cell)) {
    return cell;
  }
  // Select random cell, find all cells in the minimal bounding
  // box such that the random cell is on the edge.
  // Repeat until there is only one cell in the bounding box.
  CellSet candidateSet = _liveCells;
  srand(time(0));
  while (candidateSet.size() > 1) {
    int index = rand() % candidateSet.size();
    CellSet::iterator it = candidateSet.begin();
    for (int i = 0; i < index; ++i) {
      ++it;
      assert(it != candidateSet.end());
    }
    Cell nextCandidate = *it;
    unsigned long boxWidth = (nextCandidate.x > cell.x) ?
                             nextCandidate.x - cell.x :
                             cell.x - nextCandidate.x;
    unsigned long boxHeight = (nextCandidate.y > cell.y) ?
                              nextCandidate.y - cell.y :
                              cell.y - nextCandidate.y;
    unsigned long boxX = cell.x >= (0 + boxWidth) ? cell.x - boxWidth : 0;
    unsigned long boxY = cell.y >= (0 + boxHeight) ? cell.y - boxHeight : 0;

    candidateSet.clear();
    _quadTree.FindPoints(BoundingBox(boxX, boxY,
                                     boxWidth * 2, boxHeight * 2),
                         candidateSet);
  }
  assert(candidateSet.size() == 1);
  return *candidateSet.begin();
  
}


void
GameBoard::MarkAlive(const CellSet& cells,
                     QuadTree& tree) {
  tree.Clear();
  for (CellSet::iterator it = cells.begin();
       it != cells.end(); ++it) {
    tree.Insert(*it);
  } 
}


void
GameBoard::Reset() {
  _liveCells = _initialCells; 
  MarkAlive(_liveCells, _quadTree);
}


void
GameBoard::ChangeCell(const Cell& cell) {
  if (_changedCells.count(cell) == 0) {
    // First insert
    if (_liveCells.count(cell) == 0) {
      _changedCells.insert(cell);
      _changeQuadTree.Insert(cell);
    } else {
      _changedCells.insert(Cell(cell.x, cell.y, false));
    }
  } else {
    // Back-out of insert
    CellSet::iterator it = _changedCells.find(cell);
    assert(it != _changedCells.end());
    _changedCells.erase(it);
    MarkAlive(_changedCells, _changeQuadTree);
  }
}


void
GameBoard::CommitChanges() {
  for (CellSet::iterator it = _changedCells.begin();
       it != _changedCells.end(); ++it) {
    if (it->isAlive) {
      _liveCells.insert(*it);
    } else {
      CellSet::iterator liveIt = _liveCells.find(*it);
      // If there's a DELETE change the cell should have been alive
      assert(liveIt != _liveCells.end());
      _liveCells.erase(liveIt);
    }
  }
  MarkAlive(_liveCells, _quadTree);
  _changedCells.clear();
  _changeQuadTree.Clear();
}


void
GameBoard::UndoChanges() {
  _changedCells.clear();
  _changeQuadTree.Clear();
}


void
GameBoard::UndoPattern() {
  _pattern.clear();
  _patternQuadTree.Clear();
}


int
GameBoard::NumNeighbours(const Cell& cell) {
  BoundingBox searchBox;
  if (cell.x == 0) {
    searchBox._x = 0;
    searchBox._width = 1;
  } else {
    searchBox._x = cell.x - 1;
    searchBox._width = 2;
  }

  if (cell.y == 0) {
    searchBox._y = 0;
    searchBox._height = 1;
  } else {
    searchBox._y = cell.y - 1;
    searchBox._height = 2;
  }

  CellSet neighbours;
  _quadTree.FindPoints(searchBox, neighbours);
  // The original cell is in the search box, so should
  // be at least one. If a dead cell, we only look at
  // dead cells next to alive ones so there should be at least one.
  assert(neighbours.size() > 0);
  return cell.isAlive ? neighbours.size() - 1 : neighbours.size();
}


void
GameBoard::Draw(const ViewInfo& view,
                sf::RenderTarget& texture,
                bool running) const {

  if (!running) {
    for (int x = 0; x < view.GetHorizontalCells(); ++x) {
      for (int y = 0; y < view.GetVerticalCells(); ++y) {
        sf::RectangleShape shape(sf::Vector2f(view.cellSize, view.cellSize));
        shape.setFillColor(BACKGROUND_COLOUR);
        shape.setOutlineThickness(1);
        shape.setOutlineColor(GRID_COLOUR);
        shape.setPosition(x * view.cellSize, y * view.cellSize);
        texture.draw(shape);
      }
    }
  }

  CellSet liveCells;
  _quadTree.FindPoints(view.viewBox, liveCells);
  for (CellSet::iterator it = liveCells.begin();
       it != liveCells.end(); ++it) {
    // If stopped, don't draw cells that are deleted
    if (!running) {
      CellSet::iterator changesIt = _changedCells.find(*it);
      if (changesIt != _changedCells.end() && !changesIt->isAlive) {
      continue;
      }
    }
    it->Draw(view, texture, CELL_COLOUR);
  }
  if (!running) {
    CellSet changedCells;
    _changeQuadTree.FindPoints(view.viewBox, changedCells);
    for (CellSet::iterator it = changedCells.begin();
         it != changedCells.end(); ++it) {
      it->Draw(view, texture, GRID_COLOUR);
    }

    CellSet patternCells;
    _patternQuadTree.FindPoints(view.viewBox, patternCells);
    for (CellSet::iterator it = patternCells.begin();
         it != patternCells.end(); ++it) {
      it->Draw(view, texture, GRID_COLOUR);
    }
  }

}


void
GameBoard::ApplyPattern(const CellSet& pattern,
                        const Cell& refCell) {
  UndoPattern();
  // By convention - first cell will be centre
  CellSet::const_iterator it = pattern.cbegin();
  bool xOffsetNeg;
  bool yOffsetNeg;
  unsigned long xOffset = CalculateOffset(it->x, refCell.x, &xOffsetNeg);
  unsigned long yOffset = CalculateOffset(it->y, refCell.y, &yOffsetNeg);

  for (;it != pattern.cend(); ++it) {
    try {
      unsigned long newX = ApplyOffset(it->x, xOffset, xOffsetNeg);
      unsigned long newY = ApplyOffset(it->y, yOffset, yOffsetNeg);
      _pattern.insert(Cell(newX, newY));
    } catch (const out_of_range& err) {
      // Can't apply pattern
      _pattern.clear();
      break;
    }
  }
  MarkAlive(_pattern, _patternQuadTree);
}


void
GameBoard::CommitPattern() {
  for (CellSet::iterator patternIt = _pattern.begin();
       patternIt != _pattern.end(); ++patternIt) {
    _changedCells.insert(*patternIt);
  }
  MarkAlive(_changedCells, _changeQuadTree);
  _patternQuadTree.Clear();
  _pattern.clear();
}


void
GameBoard::Update() {
  CellSet nextLiveCells;
  CellQueue processQueue(_liveCells);

  while (!processQueue.Empty()) {
    Cell& cell = processQueue.Front();
    if (nextLiveCells.count(cell) == 0) {
      // Add neighbours if necessary
      if (cell.isAlive) {
        // TODO: Check for dupes? Worth it?
        if (cell.x < ULONG_MAX) {
          processQueue.Push(Cell(cell.x+1, cell.y, false));
          if (cell.y < ULONG_MAX) {
            processQueue.Push(Cell(cell.x+1, cell.y+1, false));
          }
          if (cell.y > 0) {
            processQueue.Push(Cell(cell.x+1, cell.y-1, false));
          }
        }
        if (cell.y < ULONG_MAX) {
          processQueue.Push(Cell(cell.x, cell.y+1, false));
        }
        if (cell.x > 0) {
          processQueue.Push(Cell(cell.x-1, cell.y, false));
          if (cell.y < ULONG_MAX) {
            processQueue.Push(Cell(cell.x-1, cell.y+1, false));
          }
          if (cell.y > 0) {
            processQueue.Push(Cell(cell.x-1, cell.y-1, false));
          }
        }
        if (cell.y > 0) {
          processQueue.Push(Cell(cell.x, cell.y-1, false));
        }
      }
      int numNeighbours = NumNeighbours(cell);
      if (numNeighbours == 3 || (cell.isAlive && numNeighbours == 2)) {
        cell.isAlive = true;
        nextLiveCells.insert(cell);
      }
      processQueue.Pop();
    }
  }

  _liveCells = nextLiveCells;
  MarkAlive(_liveCells, _quadTree);
}

