// Entry point. Owns the app state, input handling, and the per-frame update loop.

#include "raylib.h"
#include "grid.h"
#include "config.h"
#include "renderer.h"
#include "ui.h"
#include "theme.h"
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

// Everything the frame callback touches. Emscripten drives frame() from the
// browser event loop, so this cannot live in main()'s locals.
struct AppState {
    Config cfg;
    Grid grid;
    GridView view;
    Endpoint start;
    Endpoint goal;
    bool painting = false;
    Cell paintValue = Cell::Wall;
    SearchController controller;
    // Pinned by name so appending algorithms to the registry never moves it.
    int algoIdx = algorithmIndex("A*");

    explicit AppState(const Config& c)
        : cfg(c), grid(c.cols, c.rows), view{ kMargin, kMargin, c.cellSize } {}
};

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

    // Painting mutates the map the running search reads, so any edit invalidates
    // the current run. Without this the finished path can trace through cells
    // that became walls mid-search.
    if (hover.inside && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        app.painting = true;
        app.paintValue = grid.at(hover.x, hover.y) == Cell::Wall ? Cell::Floor : Cell::Wall;
        grid.set(hover.x, hover.y, app.paintValue);
        app.controller.reset();
    }
    if (app.painting && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && hover.inside) {
        if (grid.at(hover.x, hover.y) != app.paintValue) {
            grid.set(hover.x, hover.y, app.paintValue);
            app.controller.reset();
        }
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
    if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_C)) app.controller.reset();
    // X wipes the walls too, not just the search overlay.
    if (IsKeyPressed(KEY_X)) {
        grid.clear();
        app.controller.reset();
    }
    if (IsKeyPressed(KEY_UP))     app.controller.scaleSpeed(1.5);
    if (IsKeyPressed(KEY_DOWN))   app.controller.scaleSpeed(1.0 / 1.5);
}

// The status badge and the one-line hint under it double as the app's empty
// states: they always tell the user the next action that makes sense.
static void statusFor(const AppState& app, const char** text, Color* color, const char** hint) {
    const Pathfinder* algo = app.controller.current();
    if (!app.start.set || !app.goal.set) {
        *text = "waiting";  *color = theme::kTextMuted;
        *hint = !app.start.set ? "right-click places the start"
                               : "shift+right-click places the goal";
        return;
    }
    switch (app.controller.mode()) {
        case SearchController::Mode::Running:
            *text = "running";  *color = theme::kAccent;  *hint = "space pauses";
            return;
        case SearchController::Mode::Paused:
            *text = "paused";   *color = theme::kWarn;    *hint = "space resumes, s steps";
            return;
        case SearchController::Mode::Done:
            if (algo && algo->status() == SearchStatus::Found) {
                *text = "path found";  *color = theme::kGold;   *hint = "r clears the run";
            } else {
                *text = "no path";     *color = theme::kDanger; *hint = "no route between them";
            }
            return;
        default:
            *text = "ready";    *color = theme::kTextMuted; *hint = "space runs the search";
            return;
    }
}

static void drawPanel(const AppState& app) {
    const int panelX = app.view.originX + app.grid.width() * app.view.cellSize + kMargin;
    const int panelW = kPanelWidth - kMargin;
    const int x = panelX + theme::kPanelPad;
    const int cw = panelW - 2 * theme::kPanelPad;
    char buf[64];

    uiCard(Rectangle{ (float)panelX, 12, (float)panelW, 564 }, theme::kPanel);

    // Wordmark.
    int y = 28;
    uiText("path", x, y, theme::kFontTitle, theme::kText);
    uiText("finder", x + uiTextWidth("path", theme::kFontTitle), y,
           theme::kFontTitle, theme::kAccent);

    y += 34;
    uiSectionLabel("ALGORITHM", x, y);
    y += 20;
    for (int i = 0; i < algorithmCount(); ++i) {
        std::snprintf(buf, sizeof(buf), "%d", i + 1);
        uiListRow(buf, algorithmName(i), panelX + 8, y, panelW - 16, i == app.algoIdx);
        y += 26;
    }

    y += 16;
    uiSectionLabel("STATUS", x, y);
    y += 20;
    const char* statusText = "";
    const char* hint = "";
    Color statusColor = theme::kTextMuted;
    statusFor(app, &statusText, &statusColor, &hint);
    uiBadge(statusText, x, y, statusColor);
    y += 32;
    uiText(hint, x, y, theme::kFontSmall, theme::kTextMuted);

    y += 28;
    uiSectionLabel("SEARCH", x, y);
    y += 20;
    const Pathfinder* algo = app.controller.current();
    const SearchStats st = algo ? algo->stats() : SearchStats{};
    const struct { const char* label; int value; } rows[] = {
        { "expanded",   st.nodesExpanded },
        { "frontier",   st.frontierSize },
        { "path cells", st.pathLength },
        { "path cost",  st.pathCost },
    };
    for (const auto& row : rows) {
        std::snprintf(buf, sizeof(buf), "%d", row.value);
        uiStatRow(row.label, buf, x, y, cw);
        y += 22;
    }
    std::snprintf(buf, sizeof(buf), "%.0f/s", app.controller.stepsPerSecond());
    uiStatRow("speed", buf, x, y, cw);

    y += 36;
    uiSectionLabel("LEGEND", x, y);
    y += 20;
    const struct { const char* label; Color color; bool onFloor; } legend[] = {
        { "wall",     theme::kWall,     false },
        { "frontier", theme::kFrontier, true  },
        { "expanded", theme::kExpanded, true  },
        { "path",     theme::kPathCore, false },
        { "start",    theme::kStart,    false },
        { "goal",     theme::kGoal,     false },
    };
    for (int i = 0; i < 6; ++i) {
        const int col = i % 2;
        const int row = i / 2;
        uiLegendItem(legend[i].label, legend[i].color, legend[i].onFloor,
                     x + col * (cw / 2), y + row * 20);
    }
}

static void drawHelpBar(const AppState& app) {
    const int screenW = screenWidth(app.cfg);
    const int y = screenHeight(app.cfg) - kHelpBand + 8;
    const struct { const char* key; const char* desc; } hints[] = {
        { "space", "run" }, { "s", "step" }, { "enter", "finish" },
        { "r", "clear" }, { "x", "walls" }, { "1-6", "algorithm" },
        { "up/dn", "speed" },
    };
    int hx = kMargin;
    for (const auto& hint : hints) {
        const int advance = uiKeyHint(hint.key, hint.desc, hx, y);
        hx += advance;
        // Drop hints that would run off small windows rather than clipping them.
        if (hx > screenW - kMargin - 90) break;
    }
    // Same policy for the mouse line, drop it whole rather than clip mid-phrase.
    const char* mouseLine = "drag paints walls   right-click start   shift+right-click goal";
    if (uiTextWidth(mouseLine, theme::kFontSmall) <= screenW - 2 * kMargin) {
        uiText(mouseLine, kMargin, y + 28, theme::kFontSmall, theme::kTextMuted);
    }
}

static void frame(void* arg) {
    AppState& app = *static_cast<AppState*>(arg);

    handleInput(app);
    app.controller.update(GetFrameTime());

    const CellHit hover = screenToCell(app.view, app.grid, GetMouseX(), GetMouseY());
    const Pathfinder* algo = app.controller.current();
    const bool showSearch = algo && app.controller.mode() != SearchController::Mode::Idle;

    BeginDrawing();
    ClearBackground(theme::kWindowBg);
    drawGridFrame(app.grid, app.view);
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
    drawHelpBar(app);
    EndDrawing();
}

int main(int argc, char** argv) {
    Config cfg;
    if (!parseArgs(argc, argv, cfg)) {
        std::fprintf(stderr, "usage: pathfinder [--cols N] [--rows N] [--cell-size N]\n");
        return 1;
    }

    InitWindow(screenWidth(cfg), screenHeight(cfg), "Pathfinder");
    initUi();

    // Static so it outlives main()'s frame on web, where set_main_loop unwinds the stack.
    static AppState app{ cfg };
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

    shutdownUi();
    CloseWindow();
    return 0;
}
