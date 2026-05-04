#pragma once

#include <cstdint>

class Grid;

struct GridView {
    int originX;
    int originY;
    int cellSize;
};

struct CellHit {
    int x;
    int y;
    bool inside;
};

struct MarkerColor {
    uint8_t r, g, b, a;
};

void drawGrid(const Grid& grid, const GridView& view);

void drawCellHighlight(const GridView& view, int cellX, int cellY);

void drawCellMarker(const GridView& view, int cellX, int cellY, MarkerColor color);

CellHit screenToCell(const GridView& view, const Grid& grid, int screenX, int screenY);
