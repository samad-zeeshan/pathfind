// Grid implementation. Bounds are checked on every access so an off-grid index
// is a throw, not silent corruption.

#include "grid.h"

#include <algorithm>
#include <stdexcept>

Grid::Grid(int width, int height)
    : width_(width), height_(height), cells_(width * height, Cell::Floor) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Grid dimensions must be positive");
    }
}

Cell Grid::at(int x, int y) const {
    if (!inBounds(x, y)) {
        throw std::out_of_range("Grid::at out of bounds");
    }
    return cells_[index(x, y)];
}

void Grid::set(int x, int y, Cell c) {
    if (!inBounds(x, y)) {
        throw std::out_of_range("Grid::set out of bounds");
    }
    cells_[index(x, y)] = c;
}

void Grid::clear() {
    std::fill(cells_.begin(), cells_.end(), Cell::Floor);
}

bool Grid::inBounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < width_ && y < height_;
}
