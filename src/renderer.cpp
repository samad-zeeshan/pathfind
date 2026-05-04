#include "renderer.h"

#include "raylib.h"
#include "grid.h"

namespace {
constexpr Color kFloorColor     = { 40, 40, 48, 255 };
constexpr Color kWallColor      = { 200, 200, 210, 255 };
constexpr Color kGridLine       = { 20, 20, 24, 255 };
constexpr Color kHighlightColor = { 255, 220, 100, 90 };
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

void drawCellHighlight(const GridView& view, int cellX, int cellY) {
    const int s = view.cellSize;
    DrawRectangle(view.originX + cellX * s, view.originY + cellY * s, s, s, kHighlightColor);
}

void drawCellMarker(const GridView& view, int cellX, int cellY, MarkerColor color) {
    const int s = view.cellSize;
    const int cx = view.originX + cellX * s + s / 2;
    const int cy = view.originY + cellY * s + s / 2;
    const float r = s * 0.4f;
    DrawCircle(cx, cy, r, Color{ color.r, color.g, color.b, color.a });
}

CellHit screenToCell(const GridView& view, const Grid& grid, int screenX, int screenY) {
    const int s = view.cellSize;
    const int dx = screenX - view.originX;
    const int dy = screenY - view.originY;
    if (dx < 0 || dy < 0) return { 0, 0, false };
    const int cx = dx / s;
    const int cy = dy / s;
    if (!grid.inBounds(cx, cy)) return { cx, cy, false };
    return { cx, cy, true };
}
