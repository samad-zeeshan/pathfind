#pragma once

class Grid;

struct GridView {
    int originX;
    int originY;
    int cellSize;
};

void drawGrid(const Grid& grid, const GridView& view);
