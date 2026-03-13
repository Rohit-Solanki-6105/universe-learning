#include "raylib.h"
#include "screen.h"
#include "atom.h"
#include <cstdlib>

int main() {
    InitWindow(1200, 800, "Quantum Atomic Structure Simulator");
    SetTargetFPS(60);

    Screen screen;
    Atom atom;
    screen.init();

    while (!WindowShouldClose()) {
        
        // --- TOGGLE STATE ---
        if (IsKeyPressed(KEY_SPACE)) {
            atom.observed = !atom.observed;
        }

        // --- KEYBOARD INPUT FOR ATOMIC NUMBER ---
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= '0') && (key <= '9') && (atom.letterCount < 3)) {
                atom.inputBuf[atom.letterCount] = (char)key;
                atom.letterCount++;
                atom.inputBuf[atom.letterCount] = '\0'; 
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (atom.letterCount > 0) {
                atom.letterCount--;
                atom.inputBuf[atom.letterCount] = '\0';
            }
        }

        if (IsKeyPressed(KEY_ENTER)) {
            int newZ = atoi(atom.inputBuf);
            if (newZ > 0) {
                int newN = (newZ <= 20) ? newZ : (int)(newZ * 1.3f);
                atom.create(newZ, newN); // Generates classical AND quantum layers
            }
        }

        // --- UPDATES ---
        screen.update();
        atom.update();

        // --- DRAWING ---
        BeginDrawing();
            ClearBackground({5, 5, 10, 255}); 

            BeginMode3D(screen.camera);
                DrawGrid(20, 1.0f);
                atom.draw();
            EndMode3D();

            atom.drawHUD();
            DrawFPS(1100, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}