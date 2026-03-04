#include "ParticleGame.hpp"

int main() {
    Game game;

    InitWindow(REAL_SCREEN_WIDTH, REAL_SCREEN_HEIGHT, "Particle Game");
    SetTargetFPS(FPS);

    while (!WindowShouldClose()) {
        BeginDrawing();

        game.update();
        game.draw();

        EndDrawing();
    }

    CloseWindow();
    
    return 0;
}