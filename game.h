#ifndef __GAME_H__
#define __GAME_H__

#include <vector>

#include <SFML/Graphics.hpp>

#include "cell.h"
#include "quadtree.h"

struct ViewInfo {

  enum MoveDirection {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
  };

  enum ZoomDirection {
    ZOOM_IN,
    ZOOM_OUT,
  };

  BoundingBox viewBox;

  int cellSize;

  int screenWidth;
  int screenHeight;
  unsigned long xCentre;
  unsigned long yCentre;

  void Init(int width, int height, unsigned long x, unsigned long y);

  void Centre(unsigned long x, unsigned long y);

  inline void UpdateBox();

  inline int GetHorizontalCells() const {
    return viewBox._width;
  }

  inline int GetVerticalCells() const {
    return viewBox._height;
  }

  inline void Move(MoveDirection direction);

  inline void Zoom(ZoomDirection direction);

  void Resize(int newWidth, int newHeight);

  // Converts a position on the screen to a
  // cell coordinate
  Cell PosnToCell(int x, int y);

};

struct TextBox {
  sf::Font font;
  std::string prefix;
  std::string buffer;
  TextBox();
  void Draw(sf::RenderTarget& texture) const;
  void Clear();
};

/**
 * The board abstraction represents the entire game board. It's mostly a
 * container for the cells with some methods to manipulate them.
 */

class GameBoard {
private:
  CellSet _initialCells;
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

  void Draw(const ViewInfo& view, sf::RenderTarget& texture, bool running) const;

  void FlipCell(const Cell& cell);

  Cell FindNearest(const Cell& cell) const;

  // Reset to initial set
  void Reset();

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
  TextBox _inputBuffer;

  GameBoard _gameBoard;

  ViewInfo _view;

public:
  Game(const CellSet& startingPoints)
    : _gameBoard(startingPoints) {}

  void Draw(sf::RenderWindow& window) const;

  void Start();
};

#endif
