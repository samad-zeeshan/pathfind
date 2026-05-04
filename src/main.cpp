#include "raylib.h"
#include "grid.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    Grid grid(40, 30);  // a 40x30 grid, all floor

    InitWindow(screenWidth, screenHeight, "Pathfind");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Grid created, not yet drawn", 20, 20, 24, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}