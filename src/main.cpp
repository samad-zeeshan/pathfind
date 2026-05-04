#include "raylib.h"
#include "grid.h"
#include "renderer.h"
#include "astar.h"

#include <algorithm>
#include <cstdio>
#include <iterator>
#include <memory>

struct Endpoint {
    int x = 0;
    int y = 0;
    bool set = false;
};

struct Speed {
    int framesPerStep;   // wait this many frames between bursts
    int stepsPerFrame;   // then run this many expansions
    const char* label;
};

constexpr Speed kSpeeds[] = {
    { 12, 1, "very slow" },
    {  4, 1, "slow"      },
    {  1, 1, "medium"    },
    {  1, 4, "fast"      },
    {  1, 32, "very fast"},
};

constexpr MarkerColor kWhite  = { 235, 235, 240, 255 };
constexpr MarkerColor kGold   = { 255, 215, 0, 255 };
constexpr MarkerColor kRed    = { 230, 80, 80, 255 };
constexpr MarkerColor kMuted  = { 160, 160, 170, 255 };

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    Grid grid(40, 30);
    GridView view{ 20, 20, 18 };

    Endpoint start;
    Endpoint goal;

    InitWindow(screenWidth, screenHeight, "Pathfind");
    SetTargetFPS(60);
    initRenderer();

    bool painting = false;
    Cell paintValue = Cell::Wall;

    std::unique_ptr<AStarSearch> search;
    int speedIdx     = 1;   // default: slow
    int frameCounter = 0;

    while (!WindowShouldClose()) {
        const CellHit hover = screenToCell(view, grid, GetMouseX(), GetMouseY());
        const bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        if (hover.inside && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            grid.set(hover.x, hover.y, Cell::Floor);
            Endpoint& target = shift ? goal : start;
            target = { hover.x, hover.y, true };
        }

        if (hover.inside && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            painting = true;
            paintValue = grid.at(hover.x, hover.y) == Cell::Wall ? Cell::Floor : Cell::Wall;
            grid.set(hover.x, hover.y, paintValue);
        }
        if (painting && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && hover.inside) {
            grid.set(hover.x, hover.y, paintValue);
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            painting = false;
        }

        if (IsKeyPressed(KEY_SPACE) && start.set && goal.set) {
            search       = std::make_unique<AStarSearch>(grid, start.x, start.y, goal.x, goal.y);
            frameCounter = 0;
        }
        if (IsKeyPressed(KEY_ENTER) && search) {
            search->runToEnd();
        }
        if (IsKeyPressed(KEY_C)) {
            search.reset();
        }
        if (IsKeyPressed(KEY_UP)) {
            speedIdx = std::min((int)std::size(kSpeeds) - 1, speedIdx + 1);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            speedIdx = std::max(0, speedIdx - 1);
        }

        if (search && search->status() == SearchStatus::Running) {
            const Speed& sp = kSpeeds[speedIdx];
            if (++frameCounter >= sp.framesPerStep) {
                frameCounter = 0;
                for (int i = 0; i < sp.stepsPerFrame && search->status() == SearchStatus::Running; ++i) {
                    search->step();
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);
        drawGrid(grid, view);
        if (search) drawSearchState(grid, *search, view);
        if (search && search->status() == SearchStatus::Found) {
            drawPath(view, search->path());
        }
        if (start.set) drawCellMarker(view, start.x, start.y, { 80, 220, 120, 255 });
        if (goal.set)  drawCellMarker(view, goal.x,  goal.y,  { 230, 80, 80, 255 });
        if (hover.inside) drawCellHighlight(view, hover.x, hover.y);

        const int textBaseY = screenHeight - 50;
        drawUiText("L-drag walls   R-click start   Shift+R-click goal   Space run   Enter skip   Up/Down speed   C clear",
                   20, textBaseY + 28, 13, kMuted);

        char buf[160];
        std::snprintf(buf, sizeof(buf), "Speed: %s", kSpeeds[speedIdx].label);
        drawUiText(buf, 20, 20, 16, kWhite);

        if (search) {
            const char* statusText = "Running";
            MarkerColor statusColor = kWhite;
            if (search->status() == SearchStatus::Found)  { statusText = "Found";   statusColor = kGold; }
            if (search->status() == SearchStatus::NoPath) { statusText = "No path"; statusColor = kRed;  }

            std::snprintf(buf, sizeof(buf), "A*: %s   expanded %d   path %d cells",
                          statusText,
                          search->nodesExpanded(),
                          search->status() == SearchStatus::Found ? (int)search->path().size() : 0);
            drawUiText(buf, 20, textBaseY, 16, statusColor);
        }
        EndDrawing();
    }

    shutdownRenderer();
    CloseWindow();
    return 0;
}
