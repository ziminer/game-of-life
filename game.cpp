#include <queue>
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "game.h"
#include "cellQueue.h"

using namespace std;

static const int MIN_UPDATE_TIME = 100;

static const int MAX_UPDATE_TIME = 5000;

static const int UPDATE_INCREMENT = 50;

static const int DEFAULT_UPDATE_TIME = 500;

static const int ANTI_ALIASING_LEVEL = 8;

static const string GAME_NAME = "Game of Life";

static const int DEFAULT_WINDOW_WIDTH = 800;

static const int DEFAULT_WINDOW_HEIGHT = 600;

static const int INITIAL_CELL_SIZE = 20;

static const int CELL_SIZE_INCREMENT = 5;

static const int MAX_CELL_SIZE = 100;

static const int MIN_CELL_SIZE = 5;

static const sf::Color BACKGROUND_COLOUR = sf::Color(253, 246, 227);

static const sf::Color GRID_COLOUR = sf::Color(238, 232, 213);

static const sf::Color CELL_COLOUR = sf::Color(88,110,117);

inline void ViewInfo::Move(MoveDirection direction) {
  switch (direction) {
    case MOVE_UP:
      yCentre = max(yCentre - 1, (unsigned long) 0);
      break;
    case MOVE_DOWN:
      yCentre = min(yCentre + 1, ULONG_MAX);
      break;
    case MOVE_LEFT:
      xCentre = max(xCentre - 1, (unsigned long) 0);
      break;
    case MOVE_RIGHT:
      xCentre = min(xCentre + 1, ULONG_MAX);
      break;
  }
  UpdateBox();
}

void ViewInfo::Init(int width, int height, unsigned long x, unsigned long y) {
  screenWidth = width;
  screenHeight = height;
  xCentre = x;
  yCentre = y;
  cellSize = INITIAL_CELL_SIZE;
  UpdateBox();
}

void ViewInfo::Resize(int width, int height) {
  cout << "Old width: " << screenWidth << " height: " << screenHeight << " new width: " << width << " height: " << height << endl;
  screenWidth = width;
  screenHeight = height;
  
  UpdateBox();
  cout << "New horizontal cells: " << GetHorizontalCells() << " vertical: " << GetVerticalCells() << endl;
}

void ViewInfo::Centre(unsigned long x, unsigned long y) {
  xCentre = x;
  yCentre = y;
  UpdateBox();
}

inline void ViewInfo::UpdateBox() {
  // Add 1 to width and height to get cells that
  // are only partially on-screen.
  viewBox._width = screenWidth / cellSize + 1;
  viewBox._height = screenHeight / cellSize + 1;
  viewBox._x = max(xCentre - viewBox._width / 2, (unsigned long)0);
  viewBox._y = max(yCentre - viewBox._height / 2, (unsigned long)0);
}

inline void ViewInfo::Zoom(ZoomDirection direction) {
  switch (direction) {
    case ZOOM_IN:
      cellSize = min(cellSize + CELL_SIZE_INCREMENT, MAX_CELL_SIZE);
    break;
    case ZOOM_OUT:
      cellSize = max(cellSize - CELL_SIZE_INCREMENT, MIN_CELL_SIZE);
    break;
  }
  UpdateBox();
}

Cell ViewInfo::PosnToCell(int x, int y) {
  int xDiff = (x - (screenWidth / 2)) / cellSize;
  int yDiff = (y - (screenHeight / 2)) / cellSize;

  unsigned long cellX;
  unsigned long cellY;
  unsigned long xDiffUl = abs(xDiff);
  unsigned long yDiffUl = abs(yDiff);
  if (xDiff < 0) {
    if (xDiffUl > xCentre) {
      throw out_of_range("New cell coords overflow");
    }
    // Adjust to compensate for rounding
    cellX = xCentre - xDiffUl - 1;
  } else {
    if (xCentre > (ULONG_MAX - xDiffUl)) {
      throw out_of_range("New cell coords overflow");
    }
    cellX = xCentre + xDiffUl;
  }

  if (yDiff < 0) {
    if (yDiffUl > yCentre) {
      throw out_of_range("New cell coords overflow");
    }
    // Adjust to compensate for rounding
    cellY = yCentre - yDiffUl - 1;
  } else {
    if (yCentre > (ULONG_MAX - yDiffUl)) {
      throw out_of_range("New cell coords overflow");
    }
    cellY = yCentre + yDiffUl;
  }
  return Cell(cellX, cellY);
}

TextBox::TextBox() {
  if (!font.loadFromFile("font.ttf")) {
    cerr << "Font not loading!" << endl;
  } else {
    cout << "Font loaded" << endl;
  }
  buffer.clear();
  assert(buffer.empty());
}

inline void TextBox::Clear() {
  prefix.clear();
  buffer.clear();
}

void TextBox::Draw(sf::RenderTarget& texture) const {
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

GameBoard::GameBoard(const CellSet& points)
  : _initialCells(points), _liveCells(points),
    _quadTree(BoundingBox(0, 0, ULONG_MAX, ULONG_MAX))
{
  for (CellSet::const_iterator it = points.begin();
       it != points.end(); ++it) {
    cout << "Initial point " << it->x << " " << it->y << endl;
    _quadTree.Insert(*it);
  } 
}

Cell GameBoard::FindNearest(const Cell& cell) const {
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
    _quadTree.FindPoints(BoundingBox(boxX, boxY, boxWidth * 2, boxHeight * 2), candidateSet);
    cout << candidateSet.size() << endl;
  }
  assert(candidateSet.size() == 1);
  return *candidateSet.begin();
  
}

void GameBoard::KillAll() {
}

void GameBoard::MarkAlive(const CellSet& cells) {
  _quadTree.Clear();
  for (CellSet::iterator it = cells.begin();
       it != cells.end(); ++it) {
    assert(it->isAlive);
    _quadTree.Insert(*it);
  } 
}

void GameBoard::Reset() {
  _liveCells = _initialCells; 
  MarkAlive(_liveCells);
}

void GameBoard::FlipCell(const Cell& cell) {
  if (_liveCells.count(cell) == 0) {
    _quadTree.Insert(cell);
    _liveCells.insert(cell);
  } else {
    CellSet::iterator it = _liveCells.find(cell);
    assert(it != _liveCells.end());
    _liveCells.erase(it);
    MarkAlive(_liveCells);
  }
}

int GameBoard::NumNeighbours(const Cell& cell) {
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

void GameBoard::Draw(const ViewInfo& view,
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
  for (CellSet::iterator it = liveCells.begin(); it != liveCells.end(); ++it) {
    int xOffset = it->x - view.viewBox._x;
    int yOffset = it->y - view.viewBox._y;
    sf::CircleShape shape(view.cellSize/2);
    shape.setFillColor(CELL_COLOUR);
    shape.setOutlineThickness(1);
    shape.setOutlineColor(BACKGROUND_COLOUR);
    shape.setPosition(xOffset * view.cellSize, yOffset * view.cellSize);
    texture.draw(shape);
  }
}


void GameBoard::Update() {
  CellSet nextLiveCells;
  CellQueue processQueue(_liveCells);

  while (!processQueue.Empty()) {
    Cell& cell = processQueue.Front();
    if (nextLiveCells.count(cell) == 0) {
      // Add neighbours if necessary
      if (cell.isAlive) {
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
  MarkAlive(_liveCells);
}

void Game::Draw(sf::RenderWindow& window) const {
  window.clear(sf::Color(253, 246, 227));
  _gameBoard.Draw(_view, window, _running);
  _inputBuffer.Draw(window);
  window.display();
}

void Game::Start() {
  _running = true;
  sf::ContextSettings settings;
  settings.antialiasingLevel = ANTI_ALIASING_LEVEL;
  sf::RenderWindow window(
    sf::VideoMode(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT),
    GAME_NAME,
    sf::Style::Default,
    settings);

  // Initialize view
  const sf::Vector2u& size = window.getSize();
  _view.Init(size.x, size.y, 9223372036854775800, 9223372036854775800);

  window.setVerticalSyncEnabled(true);

  int msBetweenUpdates = DEFAULT_UPDATE_TIME;

  sf::Clock clock;
  bool resized = false;
  bool collectInput = false;
  bool collectJump = false;
  bool collectCentre = false;
  while (window.isOpen()) {

    Draw(window);

    sf::Time elapsed = clock.getElapsedTime();
    int timeDiff = elapsed.asMilliseconds() - msBetweenUpdates;
    if (_running && timeDiff >= 0) {
      _gameBoard.Update();
      clock.restart();
    }

    sf::Event event;
    while (window.pollEvent(event)) {
      switch (event.type) {
        case sf::Event::Closed:
          window.close();
          break;
        case sf::Event::Resized:
          resized = true;
          break;
        case sf::Event::TextEntered:
          // Ignore non-ASCI-alpha-numeric characters, except space and dash
          if (collectInput &&
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
          if (event.key.code == sf::Keyboard::BackSpace && collectInput) {
            _inputBuffer.buffer.pop_back();
          }
          if (event.key.code == sf::Keyboard::Return && collectInput) {
            collectInput = false;
            assert(!(collectJump && collectCentre));
            if ((collectJump || collectCentre) && !_inputBuffer.buffer.empty()) {
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
                  if (collectJump) {
                    _view.Centre(xUl, yUl);
                  } else if (collectCentre) {
                    // TODO: Deal with empty board
                    const Cell& nearest = _gameBoard.FindNearest(Cell(xUl, yUl));
                    _view.Centre(nearest.x, nearest.y);
                  }
                }
              }
            } else if (collectCentre) {
              // Jump to nearest from current view
              const Cell& nearest = _gameBoard.FindNearest(Cell(_view.xCentre, _view.yCentre));
              _view.Centre(nearest.x, nearest.y);
            }
            collectCentre = false;
            collectJump = false;
            _inputBuffer.Clear();
          } else if (event.key.code == sf::Keyboard::Q && !collectInput) {
            collectInput = true;
            collectJump = true;
            _inputBuffer.prefix = "Go to nearest: ";
          } else if (event.key.code == sf::Keyboard::Space && !collectInput) {
            _running = !_running;
            clock.restart();
          } else if (event.key.code == sf::Keyboard::R && !collectInput) {
            _gameBoard.Reset();
            clock.restart(); 
            Draw(window);
          } else if (event.key.code == sf::Keyboard::Equal &&
                     event.key.shift && !collectInput) {
            msBetweenUpdates = max(msBetweenUpdates - UPDATE_INCREMENT, MIN_UPDATE_TIME);
          } else if (event.key.code == sf::Keyboard::Equal &&
                     !event.key.shift && !collectInput) {
            msBetweenUpdates = min(msBetweenUpdates + UPDATE_INCREMENT, MAX_UPDATE_TIME);
          } else if (event.key.code == sf::Keyboard::Z && !collectInput) {
            _view.Zoom(ViewInfo::ZOOM_IN);
          } else if (event.key.code == sf::Keyboard::X && !collectInput) {
            _view.Zoom(ViewInfo::ZOOM_OUT);
          } else if (event.key.code == sf::Keyboard::J && !collectInput) {
            _inputBuffer.prefix = "Centre at nearest: ";
            collectInput = true;
            collectCentre = true;
          }
          break;
        case sf::Event::MouseButtonReleased:
          if (event.mouseButton.button == sf::Mouse::Right && !_running) {
            try {
              _gameBoard.FlipCell(_view.PosnToCell(event.mouseButton.x,
                                                   event.mouseButton.y));
            } catch (const out_of_range& err) {
              cerr << "Out of range error when trying to modify cell" << endl;
            }
          } else if (event.mouseButton.button == sf::Mouse::Left) {
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
      int newWidth = window.getSize().x;
      int newHeight = window.getSize().y;
      _view.Resize(newWidth, newHeight);
      // Need to create a new window because SFML changes
      // the pixel density of a window properly after resize.
      //  i.e. width 10 for a shape maps to a different number
      //       of pixels before vs after resize
      const sf::Vector2i& prevPosn = window.getPosition();
      window.create(
        sf::VideoMode(newWidth, newHeight),
        GAME_NAME,
        sf::Style::Default,
        settings);
      window.setPosition(prevPosn);
      Draw(window);
      resized = false;
    }
  }
}

