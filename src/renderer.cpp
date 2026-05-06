// Renderer implementation. Immediate-mode drawing on raylib, stateless.

#include "renderer.h"

#include "raylib.h"
#include "grid.h"
#include "theme.h"

#include <algorithm>

namespace {

Vector2 cellCenter(const GridView& view, int cellX, int cellY) {
    const float s = (float)view.cellSize;
    return { view.originX + cellX * s + s * 0.5f,
             view.originY + cellY * s + s * 0.5f };
}

float roundnessFor(float w, float h, float radiusPx) {
    const float side = w < h ? w : h;
    if (side <= 0) return 0.0f;
    const float r = radiusPx / (side * 0.5f);
    return r > 1.0f ? 1.0f : r;
}

}  // namespace

void drawGridFrame(const Grid& grid, const GridView& view) {
    const float pad = 8.0f;
    const Rectangle rec{ view.originX - pad, view.originY - pad,
                         grid.width() * (float)view.cellSize + 2 * pad,
                         grid.height() * (float)view.cellSize + 2 * pad };
    DrawRectangleRounded(rec, roundnessFor(rec.width, rec.height, theme::kRadius),
                         8, theme::kGridBg);
    DrawRectangleRoundedLinesEx(rec, roundnessFor(rec.width, rec.height, theme::kRadius),
                                8, 1.0f, theme::kBorder);
}

void drawGrid(const Grid& grid, const GridView& view) {
    const int w = grid.width();
    const int h = grid.height();
    const int s = view.cellSize;

    DrawRectangle(view.originX, view.originY, w * s, h * s, theme::kFloor);

    for (int x = 1; x < w; ++x) {
        const int px = view.originX + x * s;
        DrawLine(px, view.originY, px, view.originY + h * s, theme::kGridLine);
    }
    for (int y = 1; y < h; ++y) {
        const int py = view.originY + y * s;
        DrawLine(view.originX, py, view.originX + w * s, py, theme::kGridLine);
    }

    // Walls are inset a pixel and rounded so dense clusters still read as
    // individual cells instead of one smear.
    const float inset = s >= 10 ? 1.0f : 0.0f;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (grid.at(x, y) != Cell::Wall) continue;
            const Rectangle rec{ view.originX + x * s + inset,
                                 view.originY + y * s + inset,
                                 s - 2 * inset, s - 2 * inset };
            DrawRectangleRounded(rec, 0.22f, 4, theme::kWall);
        }
    }
}

void drawCellHighlight(const GridView& view, int cellX, int cellY) {
    const float s = (float)view.cellSize;
    // The 1px inset would make a zero or negative rect at tiny cell sizes, so
    // drop it below 6px where the outline would not read anyway.
    const float inset = s >= 6 ? 1.0f : 0.0f;
    const Rectangle rec{ view.originX + cellX * s + inset, view.originY + cellY * s + inset,
                         s - 2 * inset, s - 2 * inset };
    Color wash = theme::kAccent;
    wash.a = 28;
    DrawRectangleRounded(rec, 0.25f, 4, wash);
    Color line = theme::kAccent;
    line.a = 200;
    DrawRectangleRoundedLinesEx(rec, 0.25f, 4, 1.5f, line);
}

void drawSearchState(const Grid& grid, const Pathfinder& search, const GridView& view) {
    const int s = view.cellSize;
    for (int y = 0; y < grid.height(); ++y) {
        for (int x = 0; x < grid.width(); ++x) {
            const int px = view.originX + x * s;
            const int py = view.originY + y * s;
            if (search.isClosed({ x, y })) {
                DrawRectangle(px, py, s, s, theme::kExpanded);
            } else if (search.isOpen({ x, y })) {
                DrawRectangle(px, py, s, s, theme::kFrontier);
            }
        }
    }
}

void drawPath(const GridView& view, const std::vector<Point>& path) {
    if (path.empty()) return;
    const float s = (float)view.cellSize;

    if (path.size() == 1) {
        DrawCircleV(cellCenter(view, path[0].x, path[0].y), s * 0.22f, theme::kPathCore);
        return;
    }

    // Two passes over the same polyline: a wide halo, then the solid core. The
    // halo is opaque (precomposited in theme.h) so overlapping joints stay flat.
    // Circles at every joint round off the elbows DrawLineEx leaves.
    const float glowR = s * 0.30f;
    const float coreR = s * 0.14f;
    for (size_t k = 0; k < path.size(); ++k) {
        const Vector2 c = cellCenter(view, path[k].x, path[k].y);
        DrawCircleV(c, glowR, theme::kPathGlow);
        if (k > 0) {
            const Vector2 p = cellCenter(view, path[k - 1].x, path[k - 1].y);
            DrawLineEx(p, c, glowR * 2, theme::kPathGlow);
        }
    }
    for (size_t k = 0; k < path.size(); ++k) {
        const Vector2 c = cellCenter(view, path[k].x, path[k].y);
        DrawCircleV(c, coreR, theme::kPathCore);
        if (k > 0) {
            const Vector2 p = cellCenter(view, path[k - 1].x, path[k - 1].y);
            DrawLineEx(p, c, coreR * 2, theme::kPathCore);
        }
    }
}

void drawStartMarker(const GridView& view, int cellX, int cellY) {
    const Vector2 c = cellCenter(view, cellX, cellY);
    const float s = (float)view.cellSize;
    DrawCircleV(c, s * 0.30f, theme::kStart);
}

void drawGoalMarker(const GridView& view, int cellX, int cellY) {
    const Vector2 c = cellCenter(view, cellX, cellY);
    const float s = (float)view.cellSize;
    DrawRing(c, s * 0.20f, s * 0.34f, 0, 360, 24, theme::kGoal);
    DrawCircleV(c, s * 0.08f, theme::kGoal);
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
