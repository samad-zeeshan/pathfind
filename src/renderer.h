// Grid-space drawing: the board, search overlay, path, and markers.
// Panel and text components live in ui.h.

#pragma once

#include "pathfinder.h"

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

void drawGridFrame(const Grid& grid, const GridView& view);  // backing card, draw first
void drawGrid(const Grid& grid, const GridView& view);
void drawCellHighlight(const GridView& view, int cellX, int cellY);
void drawStartMarker(const GridView& view, int cellX, int cellY);
void drawGoalMarker(const GridView& view, int cellX, int cellY);
void drawPath(const GridView& view, const std::vector<Point>& path);
void drawSearchState(const Grid& grid, const Pathfinder& search, const GridView& view);

CellHit screenToCell(const GridView& view, const Grid& grid, int screenX, int screenY);
