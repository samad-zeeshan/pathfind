// Entry point. Owns the app state, input handling, and the per-frame update loop.

#include "raylib.h"
#include "grid.h"
#include "renderer.h"
#include "pathfinder.h"
#include "algorithms.h"
#include "controller.h"

#include <cstdio>
#include <memory>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

struct Endpoint {
    int x = 0;
    int y = 0;
    bool set = false;
};

constexpr MarkerColor kWhite = { 235, 235, 240, 255 };
constexpr MarkerColor kGold  = { 255, 215, 0, 255 };
constexpr MarkerColor kRed   = { 230, 80, 80, 255 };
constexpr MarkerColor kMuted = { 160, 160, 170, 255 };

constexpr int kPanelWidth   = 250;
constexpr int kScreenWidth  = 20 + 40 * 18 + 20 + kPanelWidth;   // grid plus side panel
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
    SearchController controller;
    int algoIdx = algorithmCount() - 1;   // default: A*, the registry's last word
};

static const char* modeLabel(SearchController::Mode mode) {
    switch (mode) {
        case SearchController::Mode::Running: return "running";
        case SearchController::Mode::Paused:  return "paused";
        case SearchController::Mode::Done:    return "done";
        default:                              return "idle";
    }
}

static void handleInput(AppState& app) {
    Grid& grid = app.grid;
    const CellHit hover = screenToCell(app.view, grid, GetMouseX(), GetMouseY());
    const bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (hover.inside && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        grid.set(hover.x, hover.y, Cell::Floor);
        Endpoint& target = shift ? app.goal : app.start;
        target = { hover.x, hover.y, true };
        if (app.start.set && app.goal.set) {
            app.controller.setEndpoints(grid, { app.start.x, app.start.y },
                                        { app.goal.x, app.goal.y });
        }
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

    // Number keys select the algorithm. The controller restarts an active run
    // under the new algorithm so the two can be compared on the same map.
    for (int i = 0; i < algorithmCount(); ++i) {
        if (IsKeyPressed(KEY_ONE + i)) {
            app.algoIdx = i;
            app.controller.select(makeAlgorithm(i));
        }
    }

    if (IsKeyPressed(KEY_SPACE))  app.controller.togglePlay();
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_PERIOD)) app.controller.stepOnce();
    if (IsKeyPressed(KEY_ENTER))  app.controller.runToEnd();
    if (IsKeyPressed(KEY_R))      app.controller.reset();
    if (IsKeyPressed(KEY_C))      app.controller.reset();
    if (IsKeyPressed(KEY_UP))     app.controller.scaleSpeed(1.5);
    if (IsKeyPressed(KEY_DOWN))   app.controller.scaleSpeed(1.0 / 1.5);
}

static void drawPanel(const AppState& app) {
    const int x = app.view.originX + app.grid.width() * app.view.cellSize + 20;
    const Pathfinder* algo = app.controller.current();
    char buf[128];
    int y = 20;

    drawUiText(algo ? algo->name() : "-", x, y, 20, kWhite);
    y += 30;

    MarkerColor modeColor = kWhite;
    if (app.controller.mode() == SearchController::Mode::Done && algo) {
        modeColor = algo->status() == SearchStatus::Found ? kGold : kRed;
        std::snprintf(buf, sizeof(buf), "done, %s",
                      algo->status() == SearchStatus::Found ? "path found" : "no path");
    } else {
        std::snprintf(buf, sizeof(buf), "%s", modeLabel(app.controller.mode()));
    }
    drawUiText(buf, x, y, 16, modeColor);
    y += 24;

    std::snprintf(buf, sizeof(buf), "%.0f steps/s", app.controller.stepsPerSecond());
    drawUiText(buf, x, y, 14, kMuted);
    y += 34;

    const SearchStats st = algo ? algo->stats() : SearchStats{};
    const struct { const char* label; int value; } rows[] = {
        { "expanded", st.nodesExpanded },
        { "frontier", st.frontierSize },
        { "path cells", st.pathLength },
        { "path cost", st.pathCost },
    };
    for (const auto& row : rows) {
        std::snprintf(buf, sizeof(buf), "%-11s %d", row.label, row.value);
        drawUiText(buf, x, y, 15, kWhite);
        y += 22;
    }
    y += 18;

    drawLegend(x, y);
}

static void frame(void* arg) {
    AppState& app = *static_cast<AppState*>(arg);

    handleInput(app);
    app.controller.update(GetFrameTime());

    const CellHit hover = screenToCell(app.view, app.grid, GetMouseX(), GetMouseY());
    const Pathfinder* algo = app.controller.current();
    const bool showSearch = algo && app.controller.mode() != SearchController::Mode::Idle;

    BeginDrawing();
    ClearBackground(BLACK);
    drawGrid(app.grid, app.view);
    if (showSearch) {
        drawSearchState(app.grid, *algo, app.view);
        if (algo->status() == SearchStatus::Found) {
            drawPath(app.view, algo->path());
        }
    }
    if (app.start.set) drawStartMarker(app.view, app.start.x, app.start.y);
    if (app.goal.set)  drawGoalMarker(app.view, app.goal.x, app.goal.y);
    if (hover.inside)  drawCellHighlight(app.view, hover.x, hover.y);

    drawPanel(app);

    const int helpY = kScreenHeight - 44;
    drawUiText("L-drag walls   R-click start   Shift+R-click goal   1-6 algorithm",
               20, helpY, 13, kMuted);
    drawUiText("Space play/pause   S step   Enter finish   R reset   C clear   Up/Down speed",
               20, helpY + 20, 13, kMuted);
    EndDrawing();
}

int main() {
    InitWindow(kScreenWidth, kScreenHeight, "Pathfind");
    initRenderer();

    // Static so it outlives main()'s frame on web, where set_main_loop unwinds the stack.
    static AppState app;
    app.controller.select(makeAlgorithm(app.algoIdx));

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
