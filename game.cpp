#include <queue>
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <fstream>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "game.h"
#include "utils.h"

using namespace std;

static const int MIN_UPDATE_TIME = 100;

static const int MAX_UPDATE_TIME = 5000;

static const int UPDATE_INCREMENT = 50;

static const int DEFAULT_UPDATE_TIME = 500;

static const int ANTI_ALIASING_LEVEL = 8;

static const string GAME_NAME = "Game of Life";

static const int DEFAULT_WINDOW_WIDTH = 800;

static const int DEFAULT_WINDOW_HEIGHT = 600;

static const sf::Color BACKGROUND_COLOUR = sf::Color(253, 246, 227);

static const unsigned long INIT_X_COORD = 9223372036854775800;

static const unsigned long INIT_Y_COORD = 9223372036854775800;

static const string PATTERN_CONFIG_SEP = " ";


TextBox::TextBox() {
  if (!font.loadFromFile("font.ttf")) {
    cerr << "Font not loading!" << endl;
  } else {
    cout << "Font loaded" << endl;
  }
  buffer.clear();
  assert(buffer.empty());
}


void
TextBox::Clear() {
  prefix.clear();
  buffer.clear();
}


void
TextBox::Draw(sf::RenderTarget& texture) const {
  if (buffer.empty() && prefix.empty()) {
    return;
  }
  sf::Text text;
  text.setFont(font);
  text.setString(prefix + buffer);
  text.setCharacterSize(20);
  text.setColor(sf::Color::Black);
  texture.draw(text);
}



void
Game::ApplyPatternAtMouse() {
  sf::Vector2i mousePosition = sf::Mouse::getPosition(_window);
  if (mousePosition.x >= 0 && mousePosition.x <= _view.screenWidth &&
      mousePosition.y >= 0 && mousePosition.y <= _view.screenHeight) {
    const Cell& mouseCell = _view.PosnToCell(mousePosition.x, mousePosition.y);
    _gameBoard.ApplyPattern(*_activePattern, mouseCell);
  }
}


void
Game::LoadPatterns(const string& patternFileName) {
  ifstream patternFile(patternFileName);
  string line;
  if (patternFile.is_open()) {
    CellSet patternCells;
    while (getline(patternFile, line)) {
      if (line.empty()) {
        _patterns.push_back(patternCells);
        patternCells.clear();
        continue;
      }
      size_t pos = line.find(PATTERN_CONFIG_SEP);
      if (pos == string::npos) {
        cerr << "Invalid config line. Expecting space-separated longs" << endl;
        patternCells.clear();
        break;
      } else {
        string xStr = line.substr(0, pos);
        string yStr = line.substr(pos+1);
        long x = atol(xStr.c_str());
        // TODO: (not important) what if string is 0000
        if (x == 0 && xStr != "0") {
          cerr << "Could not convert " << xStr << " to long" << endl;
          patternCells.clear();
          break;
        }
        long y = atol(yStr.c_str());
        if (y == 0 && yStr != "0") {
          cerr << "Could not convert " << yStr << " to long" << endl;
          patternCells.clear();
          break;
        }
        // Input format in signed long, but want to deal with unsigned
        // long internally so convert here.
        // TODO: What if input is unsigned long?
        unsigned long adjustedX = x + LONG_MAX + 1;
        unsigned long adjustedY = y + LONG_MAX + 1;
        patternCells.insert(Cell(adjustedX, adjustedY));
      }
    }
    if (!patternCells.empty()) {
      _patterns.push_back(patternCells);
    }
  }
}


Game::Game(const CellSet& startingPoints,
           const string& patternFileName)
  : _running(false), _collectInput(false), _collectJump(false),
    _collectCentre(false), _buildingPattern(false), _patternIndex(0),
    _gameBoard(startingPoints), _activePattern(NULL) {
  LoadPatterns(patternFileName);
  sf::ContextSettings settings;
  settings.antialiasingLevel = ANTI_ALIASING_LEVEL;
  _window.create(
    sf::VideoMode(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT),
    GAME_NAME,
    sf::Style::Default,
    settings);
  _window.setVerticalSyncEnabled(true);

  // Initialize view
  const sf::Vector2u& size = _window.getSize();
  _view.Init(size.x, size.y, INIT_X_COORD, INIT_Y_COORD);

}


void
Game::Draw() {
  _window.clear(BACKGROUND_COLOUR);
  _gameBoard.Draw(_view, _window, _running);
  _inputBuffer.Draw(_window);
  _window.display();
}


void
Game::RotateActivePattern() {
  assert(_activePattern != NULL);
  // Rotate around first element in pattern
  CellSet newPattern;
  CellSet::iterator it = _activePattern->begin();
  const Cell& origin = *it;
  newPattern.insert(origin);
  for (++it; it != _activePattern->end(); ++it) {
    try {
      bool xOffsetNeg;
      bool yOffsetNeg;
      unsigned long xOffset = CalculateOffset(origin.x, it->x, &xOffsetNeg);
      unsigned long yOffset = CalculateOffset(origin.y, it->y, &yOffsetNeg);
      bool newXOffsetNeg = !yOffsetNeg;
      bool newYOffsetNeg = xOffsetNeg;
      unsigned long xNewOffset = yOffset;
      unsigned long yNewOffset = xOffset;
      unsigned long newX = ApplyOffset(origin.x, xNewOffset, newXOffsetNeg);
      unsigned long newY = ApplyOffset(origin.y, yNewOffset, newYOffsetNeg);
      newPattern.insert(Cell(newX, newY));
    } catch (const out_of_range& err) {
      // Can't apply pattern
      newPattern.clear();
      break;
    }
  }
  if (!newPattern.empty()) {
    _activePattern->swap(newPattern);
  }
}


void Game::ActOnInput() {
  _collectInput = false;
  assert(!(_collectJump && _collectCentre));
  if ((_collectJump || _collectCentre) && !_inputBuffer.buffer.empty()) {
    size_t sep = _inputBuffer.buffer.find(" ");
    if (sep != string::npos) {
      // TODO: check for out-of-bounds
      string xStr = _inputBuffer.buffer.substr(0, sep);
      string yStr = _inputBuffer.buffer.substr(sep+1);
      long x = atol(xStr.c_str());
      long y = atol(yStr.c_str());
      // TODO: deal with strings like 0000
      if (!((xStr != "0" && x == 0) || (yStr != "0" && x == 0))) {
        // Need to adjust coords to unsigned long
        unsigned long xUl = x + LONG_MAX + 1;
        unsigned long yUl = y + LONG_MAX + 1; 
        if (_collectJump) {
          _view.Centre(xUl, yUl);
        } else if (_collectCentre) {
          // TODO: Deal with empty board
          const Cell& nearest = _gameBoard.FindNearest(Cell(xUl, yUl));
          _view.Centre(nearest.x, nearest.y);
        }
      }
    }
  } else if (_collectCentre) {
    // Jump to nearest from current view
    const Cell& nearest = _gameBoard.FindNearest(Cell(_view.xCentre,
                                                      _view.yCentre));
    _view.Centre(nearest.x, nearest.y);
  }
  _collectCentre = false;
  _collectJump = false;
  _inputBuffer.Clear();
}


void
Game::ExitBuildMode() {
  if (!_running) {
    _gameBoard.CommitChanges();
    if (_activePattern != NULL) {
      _gameBoard.UndoPattern();
      _activePattern = NULL;
    }
    _patternIndex = 0;
  }
  _running = !_running;
}


void
Game::ClearState() {
  _patternIndex = 0;
  // If placing a pattern, don't clear the other in-progress changes.
  if (!_running && _activePattern == NULL) {
    _gameBoard.UndoChanges();
  }
  if (_collectInput || _collectJump) {
    _collectInput = false;
    _collectJump = false;
    _inputBuffer.Clear();
  }
  if (_buildingPattern) {
    _buildingPattern = false;
    _patterns.pop_back();
  }
  if (_activePattern != NULL) {
    _gameBoard.UndoPattern();
    _activePattern = NULL;
  }
}


void
Game::Start() {
  _running = true;

  int msBetweenUpdates = DEFAULT_UPDATE_TIME;

  sf::Clock clock;
  bool resized = false;

  while (_window.isOpen()) {

    Draw();

    sf::Time elapsed = clock.getElapsedTime();
    int timeDiff = elapsed.asMilliseconds() - msBetweenUpdates;
    if (_running && timeDiff >= 0) {
      _gameBoard.Update();
      clock.restart();
    }

    sf::Event event;
    while (_window.pollEvent(event)) {
      switch (event.type) {
        case sf::Event::Closed:
          _window.close();
          break;
        case sf::Event::Resized:
          // Coalesce resized events into one when resize done.
          resized = true;
          break;
        case sf::Event::MouseMoved:
          if (_activePattern != NULL && !_buildingPattern) {
            assert(!_running);
            ApplyPatternAtMouse();
          }
          break;
        case sf::Event::TextEntered:
          // Ignore non-ASCI-alpha-numeric characters, except space and dash
          if (_collectInput &&
              (event.text.unicode == 32 || event.text.unicode == 45 ||
               (event.text.unicode >= 48 && event.text.unicode <= 57) ||
               (event.text.unicode >= 65 && event.text.unicode <= 90) ||
               (event.text.unicode >= 97 && event.text.unicode <= 122))) {
            _inputBuffer.buffer += static_cast<char>(event.text.unicode);
          }
          break;
        case sf::Event::KeyPressed:
          if (event.key.code == sf::Keyboard::Up) {
            _view.Move(ViewInfo::MOVE_UP);
          } else if (event.key.code == sf::Keyboard::Down) {
            _view.Move(ViewInfo::MOVE_DOWN);
          } else if (event.key.code == sf::Keyboard::Left) {
            _view.Move(ViewInfo::MOVE_LEFT);
          } else if (event.key.code == sf::Keyboard::Right) {
            _view.Move(ViewInfo::MOVE_RIGHT);
          }
          break;
        case sf::Event::KeyReleased:
          if (event.key.code == sf::Keyboard::LShift && _buildingPattern) {
            _buildingPattern = false;
            _patternIndex = 0;
          } else if (event.key.code == sf::Keyboard::BackSpace &&
                     _collectInput) {
            _inputBuffer.buffer.pop_back();
          } else if (event.key.code == sf::Keyboard::Return &&
                     _collectInput) {
            ActOnInput();
          } else if (event.key.code == sf::Keyboard::G && !_collectInput) {
            _collectInput = true;
            _collectJump = true;
            _inputBuffer.prefix = "Go to nearest: ";
          } else if (event.key.code == sf::Keyboard::Space && !_collectInput) {
            ExitBuildMode();
            clock.restart();
          } else if (event.key.code == sf::Keyboard::Escape) {
            ClearState();
          } else if (event.key.code == sf::Keyboard::R && !_collectInput) {
            _gameBoard.Reset();
            clock.restart(); 
            Draw();
          } else if (event.key.code == sf::Keyboard::Equal &&
                     event.key.shift && !_collectInput) {
            msBetweenUpdates = max(msBetweenUpdates - UPDATE_INCREMENT,
                                   MIN_UPDATE_TIME);
          } else if (event.key.code == sf::Keyboard::Equal &&
                     !event.key.shift && !_collectInput) {
            msBetweenUpdates = min(msBetweenUpdates + UPDATE_INCREMENT,
                                   MAX_UPDATE_TIME);
          } else if (event.key.code == sf::Keyboard::Z && !_collectInput) {
            _view.Zoom(ViewInfo::ZOOM_IN);
          } else if (event.key.code == sf::Keyboard::X && !_collectInput) {
            _view.Zoom(ViewInfo::ZOOM_OUT);
          } else if (event.key.code == sf::Keyboard::J && !_collectInput) {
            _inputBuffer.prefix = "Centre at nearest: ";
            _collectInput = true;
            _collectCentre = true;
          } else if (event.key.code == sf::Keyboard::E && !_collectInput &&
                     _activePattern != NULL) {
            RotateActivePattern();
            ApplyPatternAtMouse();
          } else if (event.key.code == sf::Keyboard::Tab && !_running &&
                     !_buildingPattern) {
            if (_patterns.size() > 0) {
              _activePattern = &_patterns.at(_patternIndex % _patterns.size());
              ++_patternIndex;
              ApplyPatternAtMouse();
            }
          }
          break;
        case sf::Event::MouseButtonReleased:
          if (event.mouseButton.button == sf::Mouse::Left && !_running) {
            try {
              // Building a pattern 
              if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                // No pattern in progress
                if (_activePattern == NULL) {
                  _patterns.push_back(CellSet());
                  _activePattern = &_patterns.at(_patterns.size()-1);
                  _buildingPattern = true;
                }
                if (_buildingPattern) {
                  _activePattern->insert(_view.PosnToCell(event.mouseButton.x,
                                                          event.mouseButton.y));
                  _gameBoard.ApplyPattern(*_activePattern,
                                          *(_activePattern->begin()));
                }
              } else if (_activePattern == NULL) {
                _gameBoard.ChangeCell(_view.PosnToCell(event.mouseButton.x,
                                                     event.mouseButton.y));
              } else {
                _gameBoard.CommitPattern();
                _activePattern = NULL;
              }
            } catch (const out_of_range& err) {
              cerr << "Out of range error when trying to modify cell" << endl;
            }
          } else if (event.mouseButton.button == sf::Mouse::Right) {
            try {
              const Cell& clickedCell = _view.PosnToCell(event.mouseButton.x,
                                                         event.mouseButton.y);
              _view.Centre(clickedCell.x, clickedCell.y);
            } catch (const out_of_range& err) {
              cerr << "Out of range when trying to navigate" << endl;
            }
          }
        default:
          break;
      }
    }

    if (resized) {
      int newWidth = _window.getSize().x;
      int newHeight = _window.getSize().y;
      _view.Resize(newWidth, newHeight);
      // Need to create a new window because SFML changes
      // the pixel density of a window properly after resize.
      //  i.e. width 10 for a shape maps to a different number
      //       of pixels before vs after resize
      const sf::Vector2i& prevPosn = _window.getPosition();
      _window.create(
        sf::VideoMode(newWidth, newHeight),
        GAME_NAME,
        sf::Style::Default,
        _window.getSettings());
      _window.setVerticalSyncEnabled(true);
      _window.setPosition(prevPosn);
      Draw();
      resized = false;
    }
  }
}

