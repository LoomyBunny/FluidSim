#include "pico/stdlib.h"
#include "fluid-sim.h"
#include <cstdio> // Include the C standard IO functions
#include <math.h>
#include <random>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

constexpr int FLUID_CELL = 0;
constexpr int AIR_CELL = 1;
constexpr int SOLID_CELL = 2;

class FlipFluid {
public:


    FlipFluid(float density, float width, float height, float spacing, float particleRadius, int maxParticles, float flipRatio)
        : density(density), h(spacing), flipRatio(flipRatio) {
        
        fNumX = static_cast<int>(width / spacing) + 1;
        fNumY = static_cast<int>(height / spacing) + 1;
        fNumCells = fNumX * fNumY;

        u.resize(fNumCells, 0.0f);
        v.resize(fNumCells, 0.0f);
        prevU.resize(fNumCells, 0.0f);
        prevV.resize(fNumCells, 0.0f);
        p.resize(fNumCells, 0.0f);
        s.resize(fNumCells, 1.0f);  // Fluid by default
        cellType.resize(fNumCells, AIR_CELL);

        this->maxParticles = maxParticles;
        particlePos.resize(2 * maxParticles, 0.0f);
        particleVel.resize(2 * maxParticles, 0.0f);
        numParticles = 0;
    }

    void printGrid() {
        std::cout << "\n\n\n";
        for (int j = fNumY - 1; j >= 0; j--) { // Print from top to bottom
            for (int i = 0; i < fNumX; i++) {
                int idx = i * fNumY + j;
                switch (cellType[idx]) {
                    case SOLID_CELL:
                        std::cout << "##";
                        break;
                    case AIR_CELL:
                        std::cout << "  ";
                        break;
                    case FLUID_CELL:
                        std::cout << "~~";
                        break;
                }
            }
            std::cout << "\n";
        }
    }


    void integrateParticles(float dt, float gravity) {
        for (int i = 0; i < numParticles; i++) {
            particleVel[2 * i + 1] += dt * gravity;
            particlePos[2 * i] += particleVel[2 * i] * dt;
            particlePos[2 * i + 1] += particleVel[2 * i + 1] * dt;
        }
    }

    void transferVelocitiesToGrid() {
        std::fill(u.begin(), u.end(), 0.0f);
        std::fill(v.begin(), v.end(), 0.0f);
        std::fill(p.begin(), p.end(), 0.0f);
        
        for (int i = 0; i < numParticles; i++) {
            float x = particlePos[2 * i];
            float y = particlePos[2 * i + 1];
            int xi = clamp(static_cast<int>(x / h), 0, fNumX - 2);
            int yi = clamp(static_cast<int>(y / h), 0, fNumY - 2);
            int idx = xi * fNumY + yi;

            if (cellType[idx] == AIR_CELL) {
                cellType[idx] = FLUID_CELL;
            }

            u[idx] += particleVel[2 * i];
            v[idx] += particleVel[2 * i + 1];
            p[idx] += 1.0f;
        }

        for (int i = 0; i < fNumCells; i++) {
            if (p[i] > 0.0f) {
                u[i] /= p[i];
                v[i] /= p[i];
            }
        }
    }

    void transferVelocitiesToParticles() {
        for (int i = 0; i < numParticles; i++) {
            float x = particlePos[2 * i];
            float y = particlePos[2 * i + 1];
            int xi = clamp(static_cast<int>(x / h), 0, fNumX - 2);
            int yi = clamp(static_cast<int>(y / h), 0, fNumY - 2);
            int idx = xi * fNumY + yi;

            float picU = u[idx];
            float picV = v[idx];

            float flipU = particleVel[2 * i] + (u[idx] - prevU[idx]);
            float flipV = particleVel[2 * i + 1] + (v[idx] - prevV[idx]);

            particleVel[2 * i] = (1.0f - flipRatio) * picU + flipRatio * flipU;
            particleVel[2 * i + 1] = (1.0f - flipRatio) * picV + flipRatio * flipV;
        }
    }

    void solveIncompressibility(int numIters, float dt) {
        std::vector<float> newP(fNumCells, 0.0f);

        for (int iter = 0; iter < numIters; iter++) {
            for (int i = 1; i < fNumX - 1; i++) {
                for (int j = 1; j < fNumY - 1; j++) {
                    int idx = i * fNumY + j;
                    if (cellType[idx] != FLUID_CELL) continue;

                    float div = u[idx + 1] - u[idx] + v[idx + fNumY] - v[idx];
                    float pressure = -div / 4.0f;
                    newP[idx] += pressure;

                    u[idx] -= pressure;
                    u[idx + 1] += pressure;
                    v[idx] -= pressure;
                    v[idx + fNumY] += pressure;
                }
            }
            p = newP;
        }
    }

    void simulate(float dt, float gravity, int numPressureIters) {
        integrateParticles(dt, gravity);
        transferVelocitiesToGrid();
        solveIncompressibility(numPressureIters, dt);
        transferVelocitiesToParticles();
    }

    void addParticle(float x, float y) {
        if (numParticles >= maxParticles) return;
        particlePos[2 * numParticles] = x;
        particlePos[2 * numParticles + 1] = y;
        numParticles++;
    }

private:
    int fNumX, fNumY, fNumCells, maxParticles, numParticles;
    float density, h, flipRatio;

    std::vector<float> u, v, prevU, prevV, p, s;
    std::vector<int> cellType;

    std::vector<float> particlePos;
    std::vector<float> particleVel;
};

int main() {
    stdio_init_all(); // Initialize standard IO
    float width = 3.0f, height = 3.0f, spacing = 0.1f, particleRadius = 0.2f, flipRatio = 0.9f;
    int maxParticles = 1000;

    FlipFluid fluid(1000.0f, width, height, spacing, particleRadius, maxParticles, flipRatio);

    // Initialize particles
    for (int i = 0; i < 30; i++)
        for (int j = 0; j < 30; j++)
            fluid.addParticle(0.1f + i * 0.02f, 0.1f + j * 0.02f);

    // Simulation loop
    for (int step = 0; step < 1000; step++) {
        fluid.simulate(1.0f / 60.0f, -9.81f, 50);
        fluid.printGrid();
        sleep_ms(10);
    }

    return 0;
}
