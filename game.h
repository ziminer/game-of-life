#ifndef __GAME_H__
#define __GAME_H__

#include <unordered_set>
#include <vector>
#include "quadtree.h"


/**
 * A cell as described in Conway's Game of Life.
 *
 * Just a container for the position and state.
 */

class Cell {
public:
  unsigned long x;

  unsigned long y;

  bool isAlive;

  Cell(unsigned long x, unsigned long y) : x(x), y(y), isAlive(true) {}

  Cell(unsigned long x, unsigned long y, bool isAlive) : x(x), y(y), isAlive(isAlive) {}

  Cell(const Cell& other) : x(other.x), y(other.y) , isAlive(other.isAlive) {}

  Cell& operator=(const Cell& other) {
    x = other.x;
    y = other.y;
    isAlive = other.isAlive;
    return *this;
  }

  bool operator==(const Cell& other) const {
    return x == other.x && y == other.y;
  }
};

struct CellHash {
  inline std::size_t operator()(const Cell& cell) const {
    // TODO: Better hash
    return ((unsigned long)(cell.x % UINT_MAX) * 31 + cell.y % UINT_MAX) % UINT_MAX;
  }
};


typedef std::unordered_set<Cell, CellHash> CellSet;


/**
 * The board abstraction represents the entire game board. It's mostly a
 * container for the cells with some methods to manipulate them.
 */

class GameBoard {
private:
  CellSet _liveCells;

  /*
   * Keep cells in a quad tree so that we don't have to track dead cells.
   * Tracking dead cells in a ULONG_MAX by ULONG_MAX grid would not be a
   * good idea :)
   */
  QuadTree _quadTree;

  void KillAll();

  void MarkAlive(const CellSet& cells);

  int NumNeighbours(const Cell& cell);

public:
  GameBoard(const CellSet& cells);

  void Draw();

  void Update();
};


/**
 * Main game class, containing the basic infrastructure
 * like the game loop, lists of entities,
 * basic event handlers, and so on.
 */

class Game {
private:
  bool _running;

  GameBoard _gameBoard;

public:
  Game(const CellSet& startingPoints)
    : _gameBoard(startingPoints) {}

  void Start();
};

#endif
