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
    int atomicNumber;
    int neutronCount;
    std::vector<Particle> protons;
    std::vector<Particle> neutrons;
    std::vector<Electron> electrons;
    char inputBuf[8]; 
    int letterCount;

    int n; 
    int l; 
    int m; 
    float colorScale;
    float clipX;
    float clipY;
    float clipZ;

    bool observed;
    float timePhase;
    std::vector<Vector3> quantumCloud;
    std::vector<float> cloudProbabilities;

    int activeUIField; 

    Atom();
    
    void create(int Z, int N);
    void update();
    void draw(Camera3D camera); // <-- UPDATED TO TAKE CAMERA
    void drawHUD();

    float legendreP(int l, int m, float x);
    float laguerreL(int n, int l, float x);
    float calculateProbability(float r, float theta, float phi);
    void generateQuantumCloud();
    void updateControls();
};

#endif