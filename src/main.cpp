#include "raylib.h"
#include "grid.h"
#include "renderer.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    Grid grid(40, 30);

    // A few walls so we can see the renderer is doing something.
    for (int x = 5; x < 25; ++x) grid.set(x, 10, Cell::Wall);
    for (int y = 10; y < 20; ++y) grid.set(25, y, Cell::Wall);

    GridView view{ 20, 20, 18 };

    InitWindow(screenWidth, screenHeight, "Pathfind");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        drawGrid(grid, view);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
