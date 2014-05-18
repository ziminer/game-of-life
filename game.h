#ifndef __GAME_H__
#define __GAME_H__

#include <set>
#include <vector>

class Cell {
public:
  long x;
  long y;
  bool isAlive;
  Cell(long x, long y) : x(x), y(y), isAlive(true) {}
  Cell(long x, long y, bool isAlive) : x(x), y(y), isAlive(isAlive) {}
  Cell(const Cell& other) : x(other.x), y(other.y) , isAlive(other.isAlive) {}
  Cell& operator=(const Cell& other) {
    x = other.x;
    y = other.y;
    isAlive = other.isAlive;
    return *this;
  }
  bool operator<(Cell lhs) const {
    return lhs.x < x || lhs.y < y;
  }
};

class GameBoard {
private:
  std::set<Cell> _liveCells;

  void KillAll();
  void MarkAlive(const std::set<Cell>& cells);
  int NumNeighbours(const Cell& cell);
public:
  GameBoard() {}
  GameBoard(const std::set<Cell>& cells);
  void Draw();
  void Update();
};

class Game {
private:
  bool _running;
  GameBoard _gameBoard;
public:
  Game(const std::set<Cell>& startingPoints)
    : _gameBoard(GameBoard(startingPoints)) {}
  void Start();
};

#endif
