#ifndef __GAME_BOARD_H__
#define __GAME_BOARD_H__

#include <vector>

#include <SFML/Graphics.hpp>

#include "utils.h"


/**
 * Structure to help map window view
 * onto the game board state.
 */

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

  void
  Init(int width,
       int height,
       unsigned long x,
       unsigned long y);

  void
  Centre(unsigned long x,
         unsigned long y);

  inline void
  UpdateBox(unsigned long newXCentre,
            unsigned long newYCentre,
            int newWidth,
            int newHeight,
            int newCellSize);

  inline int
  GetHorizontalCells() const {
    return viewBox._width;
  }

  inline int
  GetVerticalCells() const {
    return viewBox._height;
  }

  void
  Move(MoveDirection direction);

  void
  Zoom(ZoomDirection direction);

  void
  Resize(int newWidth,
         int newHeight);

  /*
   * Converts a position on the screen to a
   * cell coordinate.
   */
  Cell
  PosnToCell(int x,
             int y);

};


/**
 * The board abstraction represents the entire game board. It's mostly a
 * container for the cells with some methods to manipulate them.
 */

class GameBoard {
private:
  CellSet _initialCells;

  CellSet _liveCells;

  CellSet _changedCells;

  CellSet _pattern;

  /*
   * Keep cells in a quad tree so that we don't have to track dead cells.
   * Tracking dead cells in a ULONG_MAX by ULONG_MAX grid would not be a
   * good idea :)
   */

  QuadTree _quadTree;

  /*
   * Need auxiliary quad trees to store temporary cells because deletion
   * from quad tree is burdensome.
   */
  // For one-off activated/deactivated cells
  QuadTree _changeQuadTree;

  // For applications of patterns
  QuadTree _patternQuadTree;

  /*
   * Mark a set of cells as "alive" in a quad-tree. Overwrites
   * previous contents.
   */
  void
  MarkAlive(const CellSet& cells,
            QuadTree& tree);

  int
  NumNeighbours(const Cell& cell);

  int
  ActivateCell(const Cell& cell);

public:
  GameBoard(const CellSet& cells);

  void
  Draw(const ViewInfo& view,
       sf::RenderTarget& texture,
       bool running) const;

  /*
   * Flips the alive-ness of the cell.
   */
  void
  ChangeCell(const Cell& cell);

  /*
   * Applies current set of changes to the
   * board.
   */
  void
  CommitChanges();

  void
  UndoChanges();

  /*
   * Marks the cells in the pattern as temporarily alive.
   */
  void
  ApplyPattern(const CellSet& pattern,
               const Cell& refCell);

  /*
   * Applies the pattern to the set of changes.
   * NOT to the board!
   */
  void
  CommitPattern();

  void
  UndoPattern();

  Cell
  FindNearest(const Cell& cell) const;

  /*
   * Reset to initial set
   */
  void
  Reset();

  /*
   * Execute an update cycle.
   */
  void
  Update();

};

#endif
