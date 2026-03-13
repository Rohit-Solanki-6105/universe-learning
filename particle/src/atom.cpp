#include "atom.h"
#include "raymath.h"
#include "rlgl.h"
#include <cmath>
#include <cstring>

Atom::Atom() {
    letterCount = 0;
    memset(inputBuf, 0, sizeof(inputBuf));
    
    observed = false; 
    timePhase = 0.0f;
    
    // Default to a beautiful f-orbital (n=4, l=3, m=0) to show off the rings and lobes
    n = 4;
    l = 3;
    m = 0;
    colorScale = 1.0f;
    
    clipX = 0.0f; 
    clipY = 0.0f;
    clipZ = 0.0f;
    activeUIField = 0;

    create(1, 0);
}

void Atom::create(int Z, int N) {
    if (Z < 1) Z = 1;
    atomicNumber = Z;
    neutronCount = N;

    protons.clear();
    neutrons.clear();
    electrons.clear();

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

float Atom::laguerreL(int n, int l, float x) {
    int k = n - l - 1;
    int alpha = 2 * l + 1;
    if (k < 0) return 0.0f;
    if (k == 0) return 1.0f;
    float L0 = 1.0f;
    float L1 = 1.0f + alpha - x;
    for (int i = 1; i < k; i++) {
        float L2 = ((2.0f * i + 1.0f + alpha - x) * L1 - (i + alpha) * L0) / (i + 1.0f);
        L0 = L1; L1 = L2;
    }
    return L1;
}

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
    
    int targetPoints = 150000; 
    int pointsGenerated = 0;
    float maxR = n * n * 2.0f; 
    float maxProb = 0.0f;

    // Phase 1: Cartesian Grid Search for true maximum probability
    for(int i=0; i<80000; i++) {
        float x = (float)GetRandomValue(-1000, 1000) / 1000.0f * maxR;
        float y = (float)GetRandomValue(-1000, 1000) / 1000.0f * maxR;
        float z = (float)GetRandomValue(-1000, 1000) / 1000.0f * maxR;
        float r = sqrtf(x*x + y*y + z*z);
        if(r == 0) continue;

        float theta = acosf(y / r);
        float phi = atan2f(z, x);
        float p = calculateProbability(r, theta, phi); 
        if (p > maxProb) maxProb = p;
    }

    // Phase 2: Uniform Cartesian Rejection Sampling (Fixes the blobby polar clustering!)
    while (pointsGenerated < targetPoints) {
        float x = (float)GetRandomValue(-1000, 1000) / 1000.0f * maxR;
        float y = (float)GetRandomValue(-1000, 1000) / 1000.0f * maxR;
        float z = (float)GetRandomValue(-1000, 1000) / 1000.0f * maxR;
        
        float r = sqrtf(x*x + y*y + z*z);
        if (r == 0 || r > maxR) continue;

        float theta = acosf(y / r);
        float phi = atan2f(z, x);
        
        float prob = calculateProbability(r, theta, phi);
        
        // Gamma correction to puff up the thin outer lobes
        float visualProb = powf(prob / maxProb, 0.4f); 
        float randVal = (float)GetRandomValue(0, 1000) / 1000.0f;

        if (randVal < visualProb) {
            quantumCloud.push_back({x, y, z});
            cloudProbabilities.push_back(prob / maxProb); // Store true normalized heat
            pointsGenerated++;
        }
    }
}

void Atom::updateControls() { }

void Atom::update() {
    timePhase += 0.05f; 
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
        // CLASSICAL VIEW
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
        // QUANTUM VIEW (STATIC CLOUD WITH PROPER CUTAWAY)
        // ==========================================
        
        float L = n * n * 2.0f; // Visual bounds for the clipping wireframes
        
        // Draw the 3 Intersecting Clipping Squares (Just like your images!)
        if (clipX > -L || clipY > -L || clipZ > -L) {
            Color lineCol = Fade(WHITE, 0.3f);
            // X Plane
            DrawLine3D({clipX, -L, -L}, {clipX, L, -L}, lineCol); DrawLine3D({clipX, L, -L}, {clipX, L, L}, lineCol);
            DrawLine3D({clipX, L, L}, {clipX, -L, L}, lineCol);   DrawLine3D({clipX, -L, L}, {clipX, -L, -L}, lineCol);
            // Y Plane
            DrawLine3D({-L, clipY, -L}, {L, clipY, -L}, lineCol); DrawLine3D({L, clipY, -L}, {L, clipY, L}, lineCol);
            DrawLine3D({L, clipY, L}, {-L, clipY, L}, lineCol);   DrawLine3D({-L, clipY, L}, {-L, clipY, -L}, lineCol);
            // Z Plane
            DrawLine3D({-L, -L, clipZ}, {L, -L, clipZ}, lineCol); DrawLine3D({L, -L, clipZ}, {L, L, clipZ}, lineCol);
            DrawLine3D({L, L, clipZ}, {-L, L, clipZ}, lineCol);   DrawLine3D({-L, L, clipZ}, {-L, -L, clipZ}, lineCol);
        }

        rlPushMatrix();
        BeginBlendMode(BLEND_ADDITIVE);
        rlBegin(RL_QUADS); 
        
        for (size_t i = 0; i < quantumCloud.size(); i++) {
            Vector3 p = quantumCloud[i];

            // REMOVE 1/8th of the atom exactly like the reference image
            if (p.x > clipX && p.y > clipY && p.z > clipZ) continue;

            float prob = cloudProbabilities[i] * colorScale;
            float intensity = powf(prob, 0.3f); // Maps colors beautifully
            
            if (intensity > 0.8f)      rlColor4ub(255, 255, 200, 200); // Core White/Yellow
            else if (intensity > 0.5f) rlColor4ub(255, 150,  50, 180); // Mid Orange/Pink
            else if (intensity > 0.2f) rlColor4ub(200,  50, 150, 150); // Magenta Edge
            else                       rlColor4ub(100,  10, 150,  80); // Purple Dust

            // Draw a tiny billboard square
            float s = 0.04f;
            rlVertex3f(p.x - s, p.y - s, p.z);
            rlVertex3f(p.x + s, p.y - s, p.z);
            rlVertex3f(p.x + s, p.y + s, p.z);
            rlVertex3f(p.x - s, p.y + s, p.z);
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

    DrawText(TextFormat("Protons (Z): %i", atomicNumber), 25, 70, 16, RED);
    DrawText(TextFormat("Neutrons (N): %i", neutronCount), 25, 90, 16, LIGHTGRAY);
    DrawText(TextFormat("Electrons: %i", (int)electrons.size()), 25, 110, 16, SKYBLUE);

    DrawText("TYPE ATOMIC #:", 25, 140, 14, GRAY);
    DrawRectangle(25, 160, 100, 30, BLACK);
    DrawRectangleLines(25, 160, 100, 30, WHITE);
    DrawText(inputBuf, 35, 165, 20, YELLOW);
    DrawText("Press ENTER", 135, 168, 14, GREEN);

    DrawLine(10, 210, 330, 210, DARKGRAY);

    DrawText("QUANTUM ORBITAL CONTROLS", 25, 220, 16, MAGENTA);
    DrawText("Click & drag sliders to adjust", 25, 240, 12, GRAY);

    Vector2 mouse = GetMousePosition();
    bool clicked = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    int startY = 260;

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

            if (val != newVal) { val = newVal; return true; }
        }

        DrawText(label, 25, y + 6, 14, LIGHTGRAY);
        DrawRectangleRec(bar, Fade(DARKGRAY, 0.8f));

        float handleX = bar.x + ((val - minV) / (maxV - minV)) * bar.width;
        DrawCircle(handleX, bar.y + 4, 6, (activeUIField == id) ? YELLOW : WHITE);

        if (isInt) DrawText(TextFormat("%i", (int)val), 260, y + 5, 16, WHITE);
        else DrawText(TextFormat("%.1f", val), 260, y + 5, 16, WHITE);

        return false;
    };

    bool regenQuantum = false;

    float fn = n; if (DrawSlider(1, "n (Principal)", fn, 1.0f, 7.0f, startY, true)) { n = fn; regenQuantum = true; }
    float fl = l; if (DrawSlider(2, "l (Azimuthal)", fl, 0.0f, (float)(n - 1), startY + 35, true)) { l = fl; regenQuantum = true; }
    float fm = m; if (DrawSlider(3, "m (Magnetic)", fm, -(float)l, (float)l, startY + 70, true)) { m = fm; regenQuantum = true; }
    
    DrawSlider(4, "Color Scale", colorScale, 0.1f, 15.0f, startY + 115, false);
    
    DrawText("CLIPPING PLANES (Volume Cutaway)", 25, startY + 160, 14, ORANGE);
    DrawSlider(5, "X Clip", clipX, -25.0f, 25.0f, startY + 180, false);
    DrawSlider(6, "Y Clip", clipY, -25.0f, 25.0f, startY + 215, false);
    DrawSlider(7, "Z Clip", clipZ, -25.0f, 25.0f, startY + 250, false);

    if (regenQuantum) {
        if (l >= n) l = n - 1;
        if (m > l) m = l;
        if (m < -l) m = -l;
        generateQuantumCloud(); 
    }
}