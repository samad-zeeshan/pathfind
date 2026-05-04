// Drawing helpers for the grid, search overlay, and UI text.

#pragma once

#include "pathfinder.h"

#include <cstdint>
#include <vector>

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

void initRenderer();
void shutdownRenderer();

void drawGrid(const Grid& grid, const GridView& view);
void drawCellHighlight(const GridView& view, int cellX, int cellY);
void drawCellMarker(const GridView& view, int cellX, int cellY, MarkerColor color);
void drawPath(const GridView& view, const std::vector<Point>& path);
void drawSearchState(const Grid& grid, const Pathfinder& search, const GridView& view);

void drawUiText(const char* text, int x, int y, int size, MarkerColor color);

CellHit screenToCell(const GridView& view, const Grid& grid, int screenX, int screenY);
