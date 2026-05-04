#pragma once

#include <cstdint>
#include <utility>
#include <vector>

class Grid;
class AStarSearch;

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
void drawPath(const GridView& view, const std::vector<std::pair<int, int>>& path);
void drawSearchState(const Grid& grid, const AStarSearch& search, const GridView& view);

void drawUiText(const char* text, int x, int y, int size, MarkerColor color);

CellHit screenToCell(const GridView& view, const Grid& grid, int screenX, int screenY);
