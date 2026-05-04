#include "renderer.h"

#include "raylib.h"
#include "grid.h"
#include "astar.h"

#include <algorithm>

namespace {

constexpr Color kFloorColor     = { 40, 40, 48, 255 };
constexpr Color kWallColor      = { 200, 200, 210, 255 };
constexpr Color kGridLine       = { 20, 20, 24, 255 };
constexpr Color kHighlightColor = { 255, 220, 100, 90 };
constexpr Color kPathColor      = { 255, 215, 0, 220 };
constexpr Color kOpenColor      = { 90, 170, 255, 110 };
constexpr Color kClosedColor    = { 110, 100, 200, 95 };

Font  g_font     = {};
bool  g_fontOwned = false;
float g_fontBaseSize = 32.0f;  // load at 2x typical render size for clean downscaling

}  // namespace

void initRenderer() {
#if defined(_WIN32)
    g_font = LoadFontEx("C:\\Windows\\Fonts\\segoeui.ttf", (int)g_fontBaseSize, nullptr, 0);
    if (g_font.texture.id != 0) {
        g_fontOwned = true;
    }
#endif
    if (g_font.texture.id == 0) {
        g_font     = GetFontDefault();
        g_fontOwned = false;
    } else {
        SetTextureFilter(g_font.texture, TEXTURE_FILTER_BILINEAR);
    }
}

void shutdownRenderer() {
    if (g_fontOwned) UnloadFont(g_font);
    g_font      = {};
    g_fontOwned = false;
}

void drawUiText(const char* text, int x, int y, int size, MarkerColor color) {
    DrawTextEx(g_font, text, Vector2{ (float)x, (float)y }, (float)size, 1.0f,
               Color{ color.r, color.g, color.b, color.a });
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

void drawSearchState(const Grid& grid, const AStarSearch& search, const GridView& view) {
    const int s = view.cellSize;
    for (int y = 0; y < grid.height(); ++y) {
        for (int x = 0; x < grid.width(); ++x) {
            const int px = view.originX + x * s;
            const int py = view.originY + y * s;
            if (search.isClosed(x, y)) {
                DrawRectangle(px, py, s, s, kClosedColor);
            } else if (search.isOpen(x, y)) {
                DrawRectangle(px, py, s, s, kOpenColor);
            }
        }
    }
}

void drawPath(const GridView& view, const std::vector<std::pair<int, int>>& path) {
    const int s = view.cellSize;
    const int inset = std::max(2, s / 5);
    const int side  = s - 2 * inset;
    for (const auto& [x, y] : path) {
        DrawRectangle(view.originX + x * s + inset,
                      view.originY + y * s + inset,
                      side, side, kPathColor);
    }
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
