#ifndef __CELL_H__
#define __CELL_H__

#include <unordered_set>
#include <vector>
#include <SFML/Graphics.hpp>

struct ViewInfo;

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

  void Draw(const ViewInfo& view, sf::RenderTarget& texture, sf::Color colour) const;

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

typedef std::vector<Cell> CellPattern;

#endif
