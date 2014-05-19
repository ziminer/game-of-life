#ifndef __GAME_H__
#define __GAME_H__

#include <vector>

#include <SFML/Graphics.hpp>

#include "cell.h"
#include "quadtree.h"


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

  void Draw(sf::RenderTarget& texture);

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
