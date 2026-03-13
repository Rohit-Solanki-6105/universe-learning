#include "atom.h"
#include "raymath.h"
#include "rlgl.h"
#include <cmath>
#include <cstring>

Atom::Atom() {
    letterCount = 0;
    memset(inputBuf, 0, sizeof(inputBuf));
    
    observed = false; // Start in quantum mode to see the cloud immediately
    timePhase = 0.0f;
    
    // Default Quantum State
    n = 4;
    l = 3;
    m = 1;
    colorScale = 1.0f;
    
    // Set default clipping to 0 as requested to see the core cutaway
    clipX = 0.0f; 
    clipY = 0.0f;
    clipZ = 0.0f;
    activeUIField = 0;

    create(1, 0); // Start with Hydrogen
}

void Atom::create(int Z, int N) {
    if (Z < 1) Z = 1;
    atomicNumber = Z;
    neutronCount = N;

    protons.clear();
    neutrons.clear();
    electrons.clear();

    // 1. Classical Nucleus
    for (int i = 0; i < Z; i++) {
        protons.push_back({ {(float)GetRandomValue(-100, 100) * 0.004f, 
                             (float)GetRandomValue(-100, 100) * 0.004f, 
                             (float)GetRandomValue(-100, 100) * 0.004f} });
    }
    for (int i = 0; i < N; i++) {
        neutrons.push_back({ {(float)GetRandomValue(-100, 100) * 0.004f, 
                              (float)GetRandomValue(-100, 100) * 0.004f, 
                              (float)GetRandomValue(-100, 100) * 0.004f} });
    }

    // 2. Classical Electrons
    int remaining = Z;
    int shellN = 1;
    while (remaining > 0) {
        int capacity = 2 * (shellN * shellN);
        int toAdd = (remaining > capacity) ? capacity : remaining;
        for (int i = 0; i < toAdd; i++) {
            Electron e;
            e.orbitRadius = shellN * 2.5f; 
            e.orbitAngle = ((360.0f / toAdd) * i) * DEG2RAD; 
            e.shellIndex = shellN;
            electrons.push_back(e);
        }
        remaining -= toAdd;
        shellN++;
    }

    generateQuantumCloud();
}

// Associated Legendre Polynomials (Angular Shape)
float Atom::legendreP(int l, int m, float x) {
    m = abs(m);
    float pmm = 1.0f;
    if (m > 0) {
        float somx2 = sqrtf((1.0f - x) * (1.0f + x));
        float fact = 1.0f;
        for (int i = 1; i <= m; i++) { pmm *= -fact * somx2; fact += 2.0f; }
    }
    if (l == m) return pmm;
    float pmmp1 = x * (2.0f * m + 1.0f) * pmm;
    if (l == m + 1) return pmmp1;
    float pll = 0.0f;
    for (int ll = m + 2; ll <= l; ll++) {
        pll = ((2.0f * ll - 1.0f) * x * pmmp1 - (ll + m - 1.0f) * pmm) / (ll - m);
        pmm = pmmp1; pmmp1 = pll;
    }
    return pll;
}

// Associated Laguerre Polynomials (Radial Nodes)
float Atom::laguerreL(int n, int l, float x) {
    int k = n - l - 1;
    int alpha = 2 * l + 1;
    float L0 = 1.0f;
    if (k == 0) return L0;
    float L1 = 1.0f + alpha - x;
    if (k == 1) return L1;
    float Lk = 0.0f;
    for (int i = 2; i <= k; i++) {
        Lk = ((2.0f * i - 1.0f + alpha - x) * L1 - (i - 1.0f + alpha) * L0) / i;
        L0 = L1; L1 = Lk;
    }
    return Lk;
}

// Real Hydrogen Wave Function Probability
float Atom::calculateProbability(float r, float theta, float phi) {
    float rho = 2.0f * r / (float)n;
    float radial = powf(rho, l) * expf(-rho / 2.0f) * laguerreL(n, l, rho);
    float angular = legendreP(l, m, cosf(theta));
    float azimuth = 1.0f;
    if (m > 0) azimuth = cosf(m * phi);
    if (m < 0) azimuth = sinf(abs(m) * phi);
    float psi = radial * angular * azimuth;
    return psi * psi; 
}

void Atom::generateQuantumCloud() {
    quantumCloud.clear();
    cloudProbabilities.clear();
    
    int targetPoints = 30000; 
    int pointsGenerated = 0;
    float maxR = n * n * 2.0f; 
    float maxProb = 0.0f;

    // Pre-pass to find peak probability
    for(int i=0; i<1000; i++) {
        float r = (float)GetRandomValue(0, (int)(maxR * 100)) / 100.0f;
        float theta = (float)GetRandomValue(0, 31415) / 10000.0f;
        float phi = (float)GetRandomValue(0, 62831) / 10000.0f;
        float p = calculateProbability(r, theta, phi);
        if (p > maxProb) maxProb = p;
    }

    // Rejection sampling
    while (pointsGenerated < targetPoints) {
        float r = (float)GetRandomValue(0, (int)(maxR * 100)) / 100.0f;
        float theta = (float)GetRandomValue(0, 31415) / 10000.0f;
        float phi = (float)GetRandomValue(0, 62831) / 10000.0f;
        float prob = calculateProbability(r, theta, phi);
        
        if (((float)GetRandomValue(0, 1000) / 1000.0f * maxProb) < prob) {
            Vector3 pos = { r * sinf(theta) * cosf(phi), r * cosf(theta), r * sinf(theta) * sinf(phi) };
            quantumCloud.push_back(pos);
            cloudProbabilities.push_back(prob / maxProb);
            pointsGenerated++;
        }
    }
}

// Unused legacy keyboard controls (moved to DrawHUD for direct slider manipulation)
void Atom::updateControls() { }

void Atom::update() {
    timePhase += 0.005f; // Slowed down the swirl slightly to look more like the image

    for (auto &e : electrons) {
        float speed = 0.04f / sqrtf((float)e.shellIndex);
        e.orbitAngle += speed;
        e.position.x = cosf(e.orbitAngle) * e.orbitRadius;
        e.position.y = 0.0f; 
        e.position.z = sinf(e.orbitAngle) * e.orbitRadius;
    }
}

void Atom::draw() {
    if (observed) {
        // ==========================================
        // CLASSICAL VIEW
        // ==========================================
        DrawSphereWires({0, 0, 0}, 0.5f + (atomicNumber * 0.015f), 12, 12, Fade(GOLD, 0.3f));

        for (const auto &p : protons) DrawSphere(p.position, 0.12f, RED);
        for (const auto &n : neutrons) DrawSphere(n.position, 0.12f, GRAY);

        int maxShell = 0;
        for (const auto &e : electrons) { if (e.shellIndex > maxShell) maxShell = e.shellIndex; }

        for (int i = 1; i <= maxShell; i++) {
            float radius = i * 2.5f;
            for (int j = 0; j < 64; j++) {
                float a1 = ((float)j / 64) * 2.0f * PI;
                float a2 = ((float)(j + 1) / 64) * 2.0f * PI;
                DrawLine3D({ cosf(a1) * radius, 0.0f, sinf(a1) * radius }, 
                           { cosf(a2) * radius, 0.0f, sinf(a2) * radius }, Fade(BLUE, 0.6f));
            }
        }

        for (const auto &e : electrons) {
            DrawSphere(e.position, 0.1f, SKYBLUE);
            DrawSphereWires(e.position, 0.12f, 6, 6, Fade(WHITE, 0.5f));
        }
    } else {
        // ==========================================
        // QUANTUM VIEW (Particle Rotation & Clipping)
        // ==========================================
        
        // 1. Draw the beautiful white structural crosshairs from your image!
        if (clipX < 99.0f || clipY < 99.0f || clipZ < 99.0f) {
            DrawLine3D({clipX, -100, clipZ}, {clipX, 100, clipZ}, Fade(WHITE, 0.5f));
            DrawLine3D({-100, clipY, clipZ}, {100, clipY, clipZ}, Fade(WHITE, 0.5f));
            DrawLine3D({clipX, clipY, -100}, {clipX, clipY, 100}, Fade(WHITE, 0.5f));
        }

        rlPushMatrix();
        BeginBlendMode(BLEND_ADDITIVE);
        rlBegin(RL_LINES); 
        
        float angle = timePhase; 
        float cosA = cosf(angle);
        float sinA = sinf(angle);

        for (size_t i = 0; i < quantumCloud.size(); i++) {
            Vector3 p = quantumCloud[i];
            
            // Particles swirl physically through the space
            Vector3 rotP;
            rotP.x = p.x * cosA - p.z * sinA;
            rotP.y = p.y;
            rotP.z = p.x * sinA + p.z * cosA;

            // 3D CLIPPING: Cut away points that cross the dynamic planes
            if (rotP.x > clipX || rotP.y > clipY || rotP.z > clipZ) continue;

            float prob = cloudProbabilities[i] * colorScale;
            
            // Replicating the color palette from your screenshot
            if (prob > 0.8f)      rlColor4ub(255, 255, 255, 200); 
            else if (prob > 0.4f) rlColor4ub(255, 255, 100, 180); 
            else if (prob > 0.1f) rlColor4ub(255,  50, 150, 120); 
            else                  rlColor4ub(150,  10, 200,  80); // Purple outer bounds

            rlVertex3f(rotP.x, rotP.y, rotP.z);
            rlVertex3f(rotP.x, rotP.y + 0.04f, rotP.z);
        }
        
        rlEnd();
        EndBlendMode();
        rlPopMatrix();
    }
}

void Atom::drawHUD() {
    DrawRectangle(10, 10, 320, 560, Fade(BLACK, 0.85f));
    DrawRectangleLines(10, 10, 320, 560, DARKGRAY);
    
    DrawText("ATOM SIMULATOR", 25, 20, 20, GOLD);
    DrawText("[SPACE] Toggle Quantum/Classical", 25, 45, 12, GREEN);

    // --- CLASSICAL UI ---
    DrawText(TextFormat("Protons (Z): %i", atomicNumber), 25, 70, 16, RED);
    DrawText(TextFormat("Neutrons (N): %i", neutronCount), 25, 90, 16, LIGHTGRAY);
    DrawText(TextFormat("Electrons: %i", (int)electrons.size()), 25, 110, 16, SKYBLUE);

    DrawText("TYPE ATOMIC #:", 25, 140, 14, GRAY);
    DrawRectangle(25, 160, 100, 30, BLACK);
    DrawRectangleLines(25, 160, 100, 30, WHITE);
    DrawText(inputBuf, 35, 165, 20, YELLOW);
    DrawText("Press ENTER", 135, 168, 14, GREEN);

    DrawLine(10, 210, 330, 210, DARKGRAY);

    // --- QUANTUM UI (LIVE SLIDERS) ---
    DrawText("QUANTUM ORBITAL CONTROLS", 25, 220, 16, MAGENTA);
    DrawText("Click & drag sliders to adjust", 25, 240, 12, GRAY);

    Vector2 mouse = GetMousePosition();
    bool clicked = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    int startY = 260;

    // Custom Lambda to draw an interactive Slider bar
    auto DrawSlider = [&](int id, const char* label, float& val, float minV, float maxV, int y, bool isInt) {
        Rectangle rec = { 25, (float)y, 270, 30 };
        Rectangle bar = { 130, (float)y + 10, 120, 8 };

        bool hover = CheckCollisionPointRec(mouse, rec);
        if (hover && clicked) {
            activeUIField = id;
            float t = (mouse.x - bar.x) / bar.width;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            float newVal = minV + t * (maxV - minV);
            if (isInt) newVal = roundf(newVal);

            if (val != newVal) {
                val = newVal;
                return true; // Value changed!
            }
        }

        DrawText(label, 25, y + 6, 14, LIGHTGRAY);
        DrawRectangleRec(bar, Fade(DARKGRAY, 0.8f));

        // Draw the visual handle
        float handleX = bar.x + ((val - minV) / (maxV - minV)) * bar.width;
        DrawCircle(handleX, bar.y + 4, 6, (activeUIField == id) ? YELLOW : WHITE);

        if (isInt) DrawText(TextFormat("%i", (int)val), 260, y + 5, 16, WHITE);
        else DrawText(TextFormat("%.1f", val), 260, y + 5, 16, WHITE);

        return false;
    };

    bool regenQuantum = false;

    // Render sliders and check if they change. Notice how min/max ranges are dynamic!
    float fn = n; if (DrawSlider(1, "n (Principal)", fn, 1.0f, 7.0f, startY, true)) { n = fn; regenQuantum = true; }
    
    // Limits l to never exceed n - 1
    float fl = l; if (DrawSlider(2, "l (Azimuthal)", fl, 0.0f, (float)(n - 1), startY + 35, true)) { l = fl; regenQuantum = true; }
    
    // Limits m to be between -l and +l
    float fm = m; if (DrawSlider(3, "m (Magnetic)", fm, -(float)l, (float)l, startY + 70, true)) { m = fm; regenQuantum = true; }
    
    DrawSlider(4, "Color Scale", colorScale, 0.1f, 3.0f, startY + 115, false);
    
    DrawText("CLIPPING PLANES", 25, startY + 160, 14, ORANGE);
    DrawSlider(5, "X Clip", clipX, -100.0f, 100.0f, startY + 180, false);
    DrawSlider(6, "Y Clip", clipY, -100.0f, 100.0f, startY + 215, false);
    DrawSlider(7, "Z Clip", clipZ, -100.0f, 100.0f, startY + 250, false);

    // If an integer slider changed, regenerate the cloud instantly
    if (regenQuantum) {
        if (l >= n) l = n - 1;
        if (m > l) m = l;
        if (m < -l) m = -l;
        generateQuantumCloud(); 
    }
}