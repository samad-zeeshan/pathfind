// Entry point. Owns the app state, input handling, and the per-frame update loop.

#include "raylib.h"
#include "grid.h"
#include "renderer.h"
#include "pathfinder.h"
#include "algorithms.h"

#include <algorithm>
#include <cstdio>
#include <iterator>
#include <memory>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

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

constexpr int kScreenWidth  = 800;
constexpr int kScreenHeight = 600;

// Everything the frame callback touches. Emscripten drives frame() from the
// browser event loop, so this cannot live in main()'s locals.
struct AppState {
    Grid grid{ 40, 30 };
    GridView view{ 20, 20, 18 };
    Endpoint start;
    Endpoint goal;
    bool painting = false;
    Cell paintValue = Cell::Wall;
    std::unique_ptr<Pathfinder> search;
    int algoIdx = 3;    // default: A*, the registry's narrative ends on it
    int speedIdx = 1;   // default: slow
    int frameCounter = 0;
};

static void startSearch(AppState& app) {
    app.search = makeAlgorithm(app.algoIdx);
    app.search->init(app.grid, { app.start.x, app.start.y }, { app.goal.x, app.goal.y });
    app.frameCounter = 0;
}

static void frame(void* arg) {
    AppState& app = *static_cast<AppState*>(arg);
    Grid& grid = app.grid;
    const GridView& view = app.view;

    const CellHit hover = screenToCell(view, grid, GetMouseX(), GetMouseY());
    const bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (hover.inside && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        grid.set(hover.x, hover.y, Cell::Floor);
        Endpoint& target = shift ? app.goal : app.start;
        target = { hover.x, hover.y, true };
    }

    if (hover.inside && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        app.painting = true;
        app.paintValue = grid.at(hover.x, hover.y) == Cell::Wall ? Cell::Floor : Cell::Wall;
        grid.set(hover.x, hover.y, app.paintValue);
    }
    if (app.painting && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && hover.inside) {
        grid.set(hover.x, hover.y, app.paintValue);
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        app.painting = false;
    }

    // Number keys select the algorithm. Switching restarts an active search so
    // the two runs can be compared on the same map.
    for (int i = 0; i < algorithmCount(); ++i) {
        if (IsKeyPressed(KEY_ONE + i)) {
            app.algoIdx = i;
            if (app.search && app.start.set && app.goal.set) startSearch(app);
        }
    }

    if (IsKeyPressed(KEY_SPACE) && app.start.set && app.goal.set) {
        startSearch(app);
    }
    if (IsKeyPressed(KEY_ENTER) && app.search) {
        app.search->runToEnd();
    }
    if (IsKeyPressed(KEY_C)) {
        app.search.reset();
    }
    if (IsKeyPressed(KEY_UP)) {
        app.speedIdx = std::min((int)std::size(kSpeeds) - 1, app.speedIdx + 1);
    }
    if (IsKeyPressed(KEY_DOWN)) {
        app.speedIdx = std::max(0, app.speedIdx - 1);
    }

    if (app.search && app.search->status() == SearchStatus::Running) {
        const Speed& sp = kSpeeds[app.speedIdx];
        if (++app.frameCounter >= sp.framesPerStep) {
            app.frameCounter = 0;
            for (int i = 0; i < sp.stepsPerFrame && app.search->status() == SearchStatus::Running; ++i) {
                app.search->step();
            }
        }
    }

    BeginDrawing();
    ClearBackground(BLACK);
    drawGrid(grid, view);
    if (app.search) drawSearchState(grid, *app.search, view);
    if (app.search && app.search->status() == SearchStatus::Found) {
        drawPath(view, app.search->path());
    }
    if (app.start.set) drawCellMarker(view, app.start.x, app.start.y, { 80, 220, 120, 255 });
    if (app.goal.set)  drawCellMarker(view, app.goal.x,  app.goal.y,  { 230, 80, 80, 255 });
    if (hover.inside) drawCellHighlight(view, hover.x, hover.y);

    const int textBaseY = kScreenHeight - 50;
    drawUiText("L-drag walls   R-click start   Shift+R-click goal   1-4 algorithm   Space run   Enter skip   Up/Down speed   C clear",
               20, textBaseY + 28, 13, kMuted);

    char buf[160];
    std::snprintf(buf, sizeof(buf), "%s   speed: %s",
                  algorithmName(app.algoIdx), kSpeeds[app.speedIdx].label);
    drawUiText(buf, 20, 20, 16, kWhite);

    if (app.search) {
        const char* statusText = "Running";
        MarkerColor statusColor = kWhite;
        if (app.search->status() == SearchStatus::Found)  { statusText = "Found";   statusColor = kGold; }
        if (app.search->status() == SearchStatus::NoPath) { statusText = "No path"; statusColor = kRed;  }

        const SearchStats st = app.search->stats();
        std::snprintf(buf, sizeof(buf), "%s: %s   expanded %d   path %d cells",
                      app.search->name(), statusText, st.nodesExpanded, st.pathLength);
        drawUiText(buf, 20, textBaseY, 16, statusColor);
    }
    EndDrawing();
}

int main() {
    InitWindow(kScreenWidth, kScreenHeight, "Pathfind");
    initRenderer();

    // Static so it outlives main()'s frame on web, where set_main_loop unwinds the stack.
    static AppState app;

#if defined(PLATFORM_WEB)
    // The browser owns the loop. 0 fps means match requestAnimationFrame.
    emscripten_set_main_loop_arg(frame, &app, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        frame(&app);
    }
#endif

    shutdownRenderer();
    CloseWindow();
    return 0;
}
