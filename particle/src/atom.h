#ifndef ATOM_H
#define ATOM_H

#include "raylib.h"
#include <vector>

struct Particle {
    Vector3 position;
};

struct Electron {
    Vector3 position;
    float orbitRadius;
    float orbitAngle;
    int shellIndex;
};

class Atom {
public:
    // --- CLASSICAL DATA ---
    int atomicNumber;
    int neutronCount;
    std::vector<Particle> protons;
    std::vector<Particle> neutrons;
    std::vector<Electron> electrons;
    char inputBuf[8]; 
    int letterCount;

    // --- QUANTUM NUMBERS ---
    int n; // Principal (Size/Energy)
    int l; // Azimuthal (Shape)
    int m; // Magnetic (Orientation)

    // --- VISUAL CONTROLS ---
    float colorScale;
    float clipX;
    float clipY;
    float clipZ;

    // --- STATE & DATA ---
    bool observed;
    float timePhase;
    std::vector<Vector3> quantumCloud;
    std::vector<float> cloudProbabilities;

    // --- UI INTERACTION ---
    int activeUIField; // 0=None, 1=n, 2=l, 3=m, 4=Color, 5=ClipX, 6=ClipY, 7=ClipZ

    Atom();
    
    // Core Functions
    void create(int Z, int N);
    void update();
    void draw();
    void drawHUD();

    // Quantum Math
    float legendreP(int l, int m, float x);
    float laguerreL(int n, int l, float x);
    float calculateProbability(float r, float theta, float phi);
    void generateQuantumCloud();
    void updateControls();
};

#endif