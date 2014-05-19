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

inline void ViewInfo::Move(MoveDirection direction) {
  switch (direction) {
    case MOVE_UP:
      viewBox._y = max(viewBox._y - 1, (unsigned long) 0);
      break;
    case MOVE_DOWN:
      viewBox._y = min(viewBox._y + 1, ULONG_MAX);
      break;
    case MOVE_LEFT:
      viewBox._x = max(viewBox._x - 1, (unsigned long) 0);
      break;
    case MOVE_RIGHT:
      viewBox._x = min(viewBox._x + 1, ULONG_MAX);
      break;
  }
}

inline void ViewInfo::Zoom(ZoomDirection direction) {
  // Zoom around centre
  switch (direction) {
    case ZOOM_IN:
      cellSize = min(cellSize + CELL_SIZE_INCREMENT, MAX_CELL_SIZE);
    break;
    case ZOOM_OUT:
      cellSize = max(cellSize - CELL_SIZE_INCREMENT, MIN_CELL_SIZE);
    break;
  }
  viewBox._width = screenWidth / cellSize;
  viewBox._height = screenHeight / cellSize;
}

Cell ViewInfo::PosnToCell(int x, int y) {
  return Cell(0,0);
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
  _quadTree.Print();
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
        shape.setFillColor(sf::Color(253,246,227));
        shape.setOutlineThickness(1);
        shape.setOutlineColor(sf::Color(238, 232, 213));
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
    shape.setFillColor(sf::Color(88,110,117));
    shape.setOutlineThickness(1);
    shape.setOutlineColor(sf::Color(253,246,227));
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
  _view.cellSize = INITIAL_CELL_SIZE;
  _view.screenWidth = size.x;
  _view.screenHeight = size.y;
  _view.viewBox._x = 9223372036854775800;
  _view.viewBox._y = 9223372036854775800;
  _view.viewBox._width = size.x / _view.cellSize;
  _view.viewBox._height = size.y / _view.cellSize;

  window.setVerticalSyncEnabled(true);

  int msBetweenUpdates = DEFAULT_UPDATE_TIME;

  sf::Clock clock;
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
          if (event.key.code == sf::Keyboard::Space) {
            _running = !_running;
            clock.restart();
          } else if (event.key.code == sf::Keyboard::R) {
            _gameBoard.Reset();
            clock.restart(); 
            Draw(window);
          } else if (event.key.code == sf::Keyboard::Equal &&
                     event.key.shift) {
            msBetweenUpdates = max(msBetweenUpdates - UPDATE_INCREMENT, MIN_UPDATE_TIME);
          } else if (event.key.code == sf::Keyboard::Equal &&
                     !event.key.shift) {
            msBetweenUpdates = min(msBetweenUpdates + UPDATE_INCREMENT, MAX_UPDATE_TIME);
          } else if (event.key.code == sf::Keyboard::Z) {
            _view.Zoom(ViewInfo::ZOOM_IN);
          } else if (event.key.code == sf::Keyboard::X) {
            _view.Zoom(ViewInfo::ZOOM_OUT);
          }
          break;
        case sf::Event::MouseButtonReleased:
          if (event.mouseButton.button == sf::Mouse::Left) {
            cout << "Left released at " << event.mouseButton.x << " " << event.mouseButton.y << endl;
            const Cell& cell = _view.PosnToCell(event.mouseButton.x,
                                                event.mouseButton.y);
          }
        default:
          break;
      }
    }
  }
}

