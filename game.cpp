#include "game.h"
#include <queue>

using namespace std;

GameBoard::GameBoard(const set<Cell>& points) : _liveCells(points) {
}

void GameBoard::KillAll() {
}

void GameBoard::MarkAlive(const set<Cell>& cells) {
}

int GameBoard::NumNeighbours(const Cell& cell) {
}

void GameBoard::Draw() {
}

void GameBoard::Update() {
  queue<Cell> processQueue;
  set<Cell> nextLiveCells;

  for (set<Cell>::const_iterator it = _liveCells.begin();
       it != _liveCells.end(); ++it) {
    processQueue.push(*it);
  }

  while (!processQueue.empty()) {
    Cell& cell = processQueue.front();
    if (nextLiveCells.count(cell) == 0) {
      int numNeighbours = NumNeighbours(cell);
      if (numNeighbours == 3) {
        nextLiveCells.insert(cell);
      }
      if (cell.isAlive) {
        processQueue.push(Cell(cell.x-1, cell.y-1));
        processQueue.push(Cell(cell.x-1, cell.y));
        processQueue.push(Cell(cell.x-1, cell.y+1));
        processQueue.push(Cell(cell.x, cell.y-1));
        processQueue.push(Cell(cell.x, cell.y+1));
        processQueue.push(Cell(cell.x+1, cell.y-1));
        processQueue.push(Cell(cell.x+1, cell.y));
        processQueue.push(Cell(cell.x+1, cell.y+1));
      }
      processQueue.pop();
    }
  }

  _liveCells = nextLiveCells;
  MarkAlive(_liveCells);
}


void Game::Start() {
  _running = true;
  while (_running) {
    _gameBoard.Draw();
    _gameBoard.Update();
  }
}

