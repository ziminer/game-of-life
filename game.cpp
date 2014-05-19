#include <queue>
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "game.h"
#include "cellQueue.h"

using namespace std;

GameBoard::GameBoard(const CellSet& points)
  : _liveCells(points), _quadTree(BoundingBox(0, 0, ULONG_MAX, ULONG_MAX))
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

void GameBoard::Draw(sf::RenderTarget& texture) {
  const sf::Vector2u& size = texture.getSize();
  const int cellSize = 20;
  const int horizontalCells = size.x / cellSize;
  const int verticalCells = size.y / cellSize;
  for (int x = 0; x < horizontalCells; ++x) {
    for (int y = 0; y < verticalCells; ++y) {
      sf::CircleShape shape(cellSize/2);
      shape.setFillColor(sf::Color::Black);
      shape.setOutlineThickness(1);
      shape.setOutlineColor(sf::Color(100, 250, 0));
      shape.setPosition(x * cellSize, y * cellSize);
      texture.draw(shape);
    }
  }
  BoundingBox box(9223372036854775800, 9223372036854775800, horizontalCells, verticalCells);
  CellSet liveCells;
  _quadTree.FindPoints(box, liveCells);
  for (CellSet::iterator it = liveCells.begin(); it != liveCells.end(); ++it) {
    int xOffset = it->x - box._x;
    int yOffset = it->y - box._y;
    sf::CircleShape shape(cellSize/2);
    shape.setFillColor(sf::Color(100, 250, 0));
    shape.setOutlineThickness(1);
    shape.setOutlineColor(sf::Color::Black);
    shape.setPosition(xOffset * cellSize, yOffset * cellSize);
    texture.draw(shape);
  }
}


void GameBoard::Update() {
  if (!_liveCells.empty()) {
    cout << "--------" << endl;
    for (CellSet::iterator it = _liveCells.begin(); it != _liveCells.end(); ++it) {
      cout << "Alive cell x: " << it->x << " y: " << it->y << endl;
    }
  }
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
  chrono::milliseconds dura(500);
  this_thread::sleep_for( dura );
}


void Game::Start() {
  _running = true;
  sf::ContextSettings settings;
  settings.antialiasingLevel = 8;
  sf::RenderWindow window(sf::VideoMode(800, 600), "Game of Life", sf::Style::Default, settings);
  window.setVerticalSyncEnabled(true);

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }

    window.clear(sf::Color::Black);

    _gameBoard.Draw(window);

    window.display();

    _gameBoard.Update();
  }
}

