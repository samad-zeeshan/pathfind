#include "renderer.h"

#include "raylib.h"
#include "grid.h"

namespace {
constexpr Color kFloorColor = { 40, 40, 48, 255 };
constexpr Color kWallColor  = { 200, 200, 210, 255 };
constexpr Color kGridLine   = { 20, 20, 24, 255 };
}

void drawGrid(const Grid& grid, const GridView& view) {
    const int w = grid.width();
    const int h = grid.height();
    const int s = view.cellSize;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int px = view.originX + x * s;
            const int py = view.originY + y * s;
            const Color c = grid.at(x, y) == Cell::Wall ? kWallColor : kFloorColor;
            DrawRectangle(px, py, s, s, c);
        }
    }

    // Gridlines on top so cell borders read clearly.
    const int totalW = w * s;
    const int totalH = h * s;
    for (int x = 0; x <= w; ++x) {
        const int px = view.originX + x * s;
        DrawLine(px, view.originY, px, view.originY + totalH, kGridLine);
    }
    for (int y = 0; y <= h; ++y) {
        const int py = view.originY + y * s;
        DrawLine(view.originX, py, view.originX + totalW, py, kGridLine);
    }
}
