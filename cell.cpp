#include "cell.h"
#include "game.h"

static const sf::Color BACKGROUND_COLOUR = sf::Color(253, 246, 227);

void Cell::Draw(const ViewInfo& view, sf::RenderTarget& texture, sf::Color colour) const {
  int xOffset = x - view.viewBox._x;
  int yOffset = y - view.viewBox._y;
  sf::CircleShape shape(view.cellSize/2);
  shape.setFillColor(colour);
  shape.setOutlineThickness(1);
  shape.setOutlineColor(BACKGROUND_COLOUR);
  shape.setPosition(xOffset * view.cellSize, yOffset * view.cellSize);
  texture.draw(shape);
}

