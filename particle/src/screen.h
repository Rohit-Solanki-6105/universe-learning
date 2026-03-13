#ifndef SCREEN_H
#define SCREEN_H

#include "raylib.h"

class Screen
{
public:

    Camera3D camera;

    float moveSpeed;
    float mouseSensitivity;
    float zoomSpeed;
    bool cursorLocked;

    Screen();

    void init();
    void update();
    void reset();
};

#endif