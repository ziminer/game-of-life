# Game of Life

Simple implementation of [Conway's Game of Life](http://en.wikipedia.org/wiki/Conway's_Game_of_Life).

### Install and Run (tested with MacOSX):
* Install [SFML](www.sfml-dev.org)
* `git clone https://://github.com/ziminer/game-of-life.git`
* `make`
* `./game-of-life`

### Controls:
#### Simulation:
* `+` to speed up the simulation.
* `=` to slow down the simulation.
* `r` to reset to initial configuration.
#### Navigation:
* Arrow keys to move.
* Right click a cell to centre the screen on the cell.
* `g` + two numbers with a space inbetween + `ENTER` to centre the screen on a coordinate.
* `j` + two numbers with a space inbetween + `ENTER` to jump to the nearest neighbour to a coordinate.
* `j` + ENTER to jump to nearest neighbour.
* `z` to zoom in.
* `x` to zoom out.
#### Build:
* `SPACE` to pause and go into build mode.
* Left click on a cell to activate/deactivate it.
* `ESC` to discard all changes.
#### Build - Patterns:
* In build mode, `TAB` to cycle through the available cell patterns.
* Drag mouse to move pattern around.
* `e` to rotate pattern.
* Left click to activate/deactivate.
* Hold `SHIFT` and left click on cells to create a pattern (finalized when you let go of `SHIFT`). This pattern will also become available via `TAB`.
* `ESC` to discard pattern.

### Known Issues:
* Clicking on cells can be buggy - some cells hard to access.

