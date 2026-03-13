#include "screen.h"
#include "raymath.h"

Screen::Screen()
{
    moveSpeed = 0.2f;
    mouseSensitivity = 0.005f;
    zoomSpeed = 1.5f;

    cursorLocked = false;
}

void Screen::init()
{
    camera.position = {6.0f, 6.0f, 6.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 45;
    camera.projection = CAMERA_PERSPECTIVE;
}

void Screen::reset()
{
    camera.position = {6, 6, 6};
    camera.target = {0, 0, 0};
}

void Screen::update()
{

    // Toggle mouse lock
    if (IsKeyPressed(KEY_TAB))
    {
        cursorLocked = !cursorLocked;

        if (cursorLocked)
            DisableCursor();
        else
            EnableCursor();
    }

    // Fullscreen
    if (IsKeyPressed(KEY_F11))
        ToggleFullscreen();

    float speed = moveSpeed;

    if (IsKeyDown(KEY_LEFT_SHIFT))
        speed *= 3;
    if (IsKeyDown(KEY_LEFT_CONTROL))
        speed *= 0.4f;

    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

    // WASD movement
    if (IsKeyDown(KEY_W))
        camera.position = Vector3Add(camera.position, Vector3Scale(forward, speed));

    if (IsKeyDown(KEY_S))
        camera.position = Vector3Subtract(camera.position, Vector3Scale(forward, speed));

    if (IsKeyDown(KEY_A))
        camera.position = Vector3Subtract(camera.position, Vector3Scale(right, speed));

    if (IsKeyDown(KEY_D))
        camera.position = Vector3Add(camera.position, Vector3Scale(right, speed));

    if (IsKeyDown(KEY_Q))
        camera.position.y += speed;

    if (IsKeyDown(KEY_E))
        camera.position.y -= speed;

    camera.target = Vector3Add(camera.position, forward);

    // ----------------
    // ORBIT CAMERA
    // ----------------
    if (IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        Vector2 delta = GetMouseDelta();

        Vector3 offset = Vector3Subtract(camera.position, camera.target);

        Matrix rotY = MatrixRotateY(-delta.x * mouseSensitivity);
        Matrix rotX = MatrixRotateX(-delta.y * mouseSensitivity);

        offset = Vector3Transform(offset, rotY);
        offset = Vector3Transform(offset, rotX);

        camera.position = Vector3Add(camera.target, offset);
    }

    // ----------------
    // FREE LOOK
    // ----------------
    if (cursorLocked && IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
    {
        Vector2 delta = GetMouseDelta();

        Matrix rotY = MatrixRotateY(-delta.x * mouseSensitivity);
        Matrix rotX = MatrixRotateX(-delta.y * mouseSensitivity);

        Vector3 forward = Vector3Subtract(camera.target, camera.position);

        forward = Vector3Transform(forward, rotY);
        forward = Vector3Transform(forward, rotX);

        camera.target = Vector3Add(camera.position, forward);
    }

    // ----------------
    // PAN
    // ----------------
    if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON))
    {
        Vector2 delta = GetMouseDelta();

        Vector3 pan =
            {
                -delta.x * 0.02f,
                delta.y * 0.02f,
                0};

        camera.position = Vector3Add(camera.position, pan);
        camera.target = Vector3Add(camera.target, pan);
    }

    // ----------------
    // ZOOM (SAFE)
    // ----------------
    // ZOOM
    float wheel = GetMouseWheelMove();

    if (wheel != 0)
    {
        Vector3 direction = Vector3Subtract(camera.position, camera.target);
        float distance = Vector3Length(direction);

        direction = Vector3Normalize(direction);

        distance -= wheel * zoomSpeed;

        // Clamp distance
        if (distance < 0.5f)
            distance = 0.5f;
        if (distance > 500.0f)
            distance = 500.0f;

        camera.position = Vector3Add(camera.target, Vector3Scale(direction, distance));
    }

    // Reset
    if (IsKeyPressed(KEY_R))
        reset();

    // Focus
    if (IsKeyPressed(KEY_F))
        camera.target = {0, 0, 0};
}