// The wall-or-floor occupancy grid every search and the renderer read.

#pragma once

#include <cstdint>
#include <vector>

enum class Cell : uint8_t {
    Floor,
    Wall
};

class Grid {
public:
    Grid(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }

    Cell at(int x, int y) const;
    void set(int x, int y, Cell c);
    void clear();   // every cell back to Floor

    bool inBounds(int x, int y) const;

private:
    int width_;
    int height_;
    std::vector<Cell> cells_;

    int index(int x, int y) const { return y * width_ + x; }
};