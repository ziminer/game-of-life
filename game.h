#ifndef __GAME_H__
#define __GAME_H__

#include <vector>

#include <SFML/Graphics.hpp>

#include "cell.h"
#include "utils.h"
#include "gameBoard.h"


/**
 * Utility structure representing
 * a text box on the screen.
 */

struct TextBox {
  sf::Font font;

  std::string prefix;

  std::string buffer;

  TextBox();

  void
  Draw(sf::RenderTarget& texture) const;

  void
  Clear();
};


/**
 * Main game class, containing the basic infrastructure
 * like the game loop, lists of entities,
 * basic event handlers, and so on.
 */

class Game {
private:
  /*
   * These variables together represent the state with
   * regard to user input.
   *
   * This is ugly. TODO: A state machine with well
   *                     defined transitions
   */
  bool _running;

  bool _collectInput;

  bool _collectJump;

  bool _collectCentre;

  bool _buildingPattern;

  int _patternIndex;

  TextBox _inputBuffer;

  GameBoard _gameBoard;

  /*
   * Builtin and user-defined configurations of cells that
   * get treated as one.
   */
  std::vector<CellSet> _patterns;

  CellSet *_activePattern;

  sf::RenderWindow _window;

  // Information about the window view.
  ViewInfo _view;

  void
  LoadPatterns(const std::string& patternFileName);

  void
  RotateActivePattern();

  void
  ApplyPatternAtMouse();

  void
  ActOnInput();

  void
  ExitBuildMode();

  /*
   * Resets the various state buffers.
   */
  void
  ClearState();

  void
  Draw();

public:
  Game(const CellSet& startingPoints,
       const std::string& patternFileName);

  void
  Start();
};

#endif
