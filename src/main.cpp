#include "raylib.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Pathfind");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Hello pathfinder", 20, 20, 24, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}