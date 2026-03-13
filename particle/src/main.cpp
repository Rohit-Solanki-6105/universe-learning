#include "raylib.h"
#include "raymath.h"
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
        
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }

        if (IsKeyPressed(KEY_SPACE)) {
            atom.observed = !atom.observed;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 delta = GetMouseDelta();
            float sens = 0.005f;
            
            Vector3 v = Vector3Subtract(screen.camera.position, screen.camera.target);
            Matrix rotY = MatrixRotateY(-delta.x * sens);
            v = Vector3Transform(v, rotY);
            
            Vector3 right = Vector3Normalize(Vector3CrossProduct(screen.camera.up, v));
            Matrix rotPitch = MatrixRotate(right, -delta.y * sens);
            v = Vector3Transform(v, rotPitch);
            
            screen.camera.position = Vector3Add(screen.camera.target, v);
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            Vector3 v = Vector3Subtract(screen.camera.position, screen.camera.target);
            float len = Vector3Length(v);
            len -= wheel * 2.5f; 
            if (len < 2.0f) len = 2.0f; 
            
            v = Vector3Scale(Vector3Normalize(v), len);
            screen.camera.position = Vector3Add(screen.camera.target, v);
        }

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
                atom.create(newZ, newN); 
            }
        }

        atom.update();

        BeginDrawing();
            ClearBackground({5, 5, 10, 255}); 

            BeginMode3D(screen.camera);
                DrawGrid(30, 1.0f);
                atom.draw(screen.camera); // <-- PASSED THE CAMERA HERE!
            EndMode3D();

            atom.drawHUD();
            
            DrawText("Right-Click & Drag to Orbit | Mouse Wheel to Zoom | F11 for Fullscreen", 350, GetScreenHeight() - 30, 16, GRAY);
            DrawFPS(GetScreenWidth() - 100, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}