#include "raylib.h"
#include "grid.h"
#include "renderer.h"

struct Endpoint {
    int x = 0;
    int y = 0;
    bool set = false;
};

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    Grid grid(40, 30);
    GridView view{ 20, 20, 18 };

    Endpoint start;
    Endpoint goal;

    InitWindow(screenWidth, screenHeight, "Pathfind");
    SetTargetFPS(60);

    bool painting = false;
    Cell paintValue = Cell::Wall;

    while (!WindowShouldClose()) {
        const CellHit hover = screenToCell(view, grid, GetMouseX(), GetMouseY());
        const bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        // Right-click: place start (or goal if shift held). Clears any wall in that cell.
        if (hover.inside && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            grid.set(hover.x, hover.y, Cell::Floor);
            Endpoint& target = shift ? goal : start;
            target = { hover.x, hover.y, true };
        }

        // Left-click drag: paint walls. First click decides whether to add or erase.
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

        BeginDrawing();
        ClearBackground(BLACK);
        drawGrid(grid, view);
        if (start.set) drawCellMarker(view, start.x, start.y, { 80, 220, 120, 255 });
        if (goal.set)  drawCellMarker(view, goal.x,  goal.y,  { 230, 80, 80, 255 });
        if (hover.inside) drawCellHighlight(view, hover.x, hover.y);

        DrawText("L-drag: walls   R-click: start   Shift+R-click: goal",
                 20, screenHeight - 28, 16, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
