#include "pico/stdlib.h"
#include "fluid-sim.h"
#include <hardware/i2c.h>
#include <hardware/clocks.h>
#include "pico/multicore.h"
#include <math.h>
#include <random>

auto highResGravityField = generateGravityField(gravityField);
int16_t x,y,z;

std::pair<float, float> getGravityForceForParticle(fluidParticle& particle) {
    // Convert particle position to high-res grid coordinates
    float highX = particle.getX() * UPSCALE;
    float highY = particle.getY() * UPSCALE;

    // Find the surrounding grid points
    int x0 = static_cast<int>(highX);
    int y0 = static_cast<int>(highY);
    int x1 = (x0 + 1 < HIGH_X) ? x0 + 1 : x0;
    int y1 = (y0 + 1 < HIGH_Y) ? y0 + 1 : y0;

    // Compute interpolation weights
    float tx = highX - x0;
    float ty = highY - y0;
    float sx = 1.0f - tx;
    float sy = 1.0f - ty;

    // Fetch forces from the four surrounding grid points
    auto [Fx00, Fy00] = highResGravityField[x0][y0];
    auto [Fx10, Fy10] = highResGravityField[x1][y0];
    auto [Fx01, Fy01] = highResGravityField[x0][y1];
    auto [Fx11, Fy11] = highResGravityField[x1][y1];

    // Bilinear interpolation
    float Fx = sx * sy * Fx00 + tx * sy * Fx10 + sx * ty * Fx01 + tx * ty * Fx11;
    float Fy = sx * sy * Fy00 + tx * sy * Fy10 + sx * ty * Fy01 + tx * ty * Fy11;

    return {Fx, Fy};  // Return as a pair
}

void dampenParticleVelocity(fluidParticle& particle, const std::array<std::array<uint8_t, ysize>, xsize>& gravityField) {
    // Get particle's position in the gravity field
    int cellX = particle.getCellX();
    int cellY = particle.getCellY();

    // Ensure the particle is within bounds
    if (cellX >= 0 && cellX < xsize && cellY >= 0 && cellY < ysize) {
        // If inside a letter (gravityField[cellX][cellY] == 1), apply damping
        if (gravityField[cellX][cellY]) {
            particle.vx *= 0.995f;  // Reduce velocity by 25%
            particle.vy *= 0.995f;
        }
    }
}


float getRandomFloat(float lower, float upper) {
    static std::random_device rd;  // Seed generator
    static std::mt19937 gen(rd()); // Mersenne Twister RNG
    std::uniform_real_distribution<float> dist(lower, upper);
    return dist(gen);
}

bool fluidCell::isSolid(){
    return state==cellStateEnum::solid;
}

bool fluidCell::isWater(){
    return state==cellStateEnum::water;
}

bool fluidCell::isAir(){
    return state==cellStateEnum::air;
}

void fluidWindow::print(){
    //printf("\n\n\n");
    for (size_t j = 0; j < ysize; j++){
        for (size_t i = xsize; i > 0; i--){
            switch (cells[i-1][j].state){
                case cellStateEnum::solid:
                    if(cordsToLedNumber[i-1][j]>255){
                        ledBuffer2[cordsToLedNumber[i-1][j]-255] = 0;
                    } else if (cordsToLedNumber[i-1][j]>0) {
                        ledBuffer1[cordsToLedNumber[i-1][j]] = 0;
                    }
                    //printf("XX");
                    break;
                case cellStateEnum::air:
                    if(cordsToLedNumber[i-1][j]>255){
                        ledBuffer2[cordsToLedNumber[i-1][j]-255] = 0;
                    } else if (cordsToLedNumber[i-1][j]>0) {
                        ledBuffer1[cordsToLedNumber[i-1][j]] = 0;
                    }
                    //printf("  ");
                    break;
                case cellStateEnum::water:
                    if(cordsToLedNumber[i-1][j]>255){
                        ledBuffer2[cordsToLedNumber[i-1][j]-255] = MIN(1+4*cells[i-1][j].numberParticles,255);
                    } else if (cordsToLedNumber[i-1][j]>0) {
                        ledBuffer1[cordsToLedNumber[i-1][j]] = MIN(1+4*cells[i-1][j].numberParticles,255);
                    }
                    //printf("~~");
                    break;
            }
        }
        //printf("%u",j);
        //printf("\n");
    }
}

fluidCell& fluidWindow::right(fluidCell& cell){
    if(cell.x + 1 >= xsize){
        //printf("Error: Right cell out of bounds\n");
        return cells[cell.x][cell.y];
    }
    return cells[cell.x+1][cell.y];
}
fluidCell& fluidWindow::left(fluidCell& cell){
    if(cell.x - 1 < 0){
        //printf("Error: Left cell out of bounds\n");
        return cells[cell.x][cell.y];
    }
    return cells[cell.x-1][cell.y];
}
fluidCell& fluidWindow::up(fluidCell& cell){
    if(cell.y -1 < 0){
        //printf("Error: Up cell out of bounds\n");
        return cells[cell.x][cell.y];
    }
    return cells[cell.x][cell.y-1];
}
fluidCell& fluidWindow::down(fluidCell& cell){
    if(cell.y + 1 >= ysize){
        //printf("Error: Down cell out of bounds\n");
        return cells[cell.x][cell.y];
    }
    return cells[cell.x][cell.y+1];
}

void fluidParticle::setCoordinates(float newX, float newY){
    if(newX<0||newX>=xsize){
        //printf("X is out of bounds! Clamping it\n");
    }
    x = clamp<float>(newX, 0, xsize-0.0001);
    cellX = floor(x);
    if(newY<0||newY>=ysize){
        //printf("Y is out of bounds! Clamping it\n");
    }
    y = clamp<float>(newY,0,ysize-0.0001);
    cellY = floor(y);
    cellNumber = cellX + xsize*cellY;
}

float fluidParticle::getX(){
    return x;
}

float fluidParticle::getY(){
    return y;
}

uint8_t fluidParticle::getCellX(){
    return cellX;
}

uint8_t fluidParticle::getCellY(){
    return cellY;
}

void fluidWindow::init(){    
    for( size_t i = 0; i < xsize; i++){
        for( size_t j = 0; j < ysize; j++){
            if(cordsToLedNumber[i][j]<0){
                cells[i][j].state = cellStateEnum::solid;
                cells[i][j].flowAllowed = 0;
            }
            cells[i][j].x = i;
            cells[i][j].y = j;
        }
    }
    // Setup the particles
    uint32_t particleId = 0;
    printf("Init coord set\n");
    for(auto& particle: particleArray){
        do{
            particle.setCoordinates(getRandomFloat(1,xsize-1.001),getRandomFloat(1,ysize-1.001));
            particle.vx = 0;
            particle.vy = 0;
            particle.particleId = particleId;
            particleId++;
        } while (cells[particle.getCellX()][particle.getCellY()].isSolid());
    }
    particleArray[0].setCoordinates(5,1.1);
}

void fluidWindow::updateDataStructures(){
    for(auto &colmn: cells){
        for(auto& cell: colmn){
            cell.numberParticles = 0;
            if(cell.isWater()){
                cell.state = cellStateEnum::air;
            }
        }
    }
    // Clear the cellParticleCount array
    for (auto &cell: cellParticleCount){
        cell = 0;
    }
    // Update the particleCell array
    for (auto &particle: particleArray){
        cellParticleCount[particle.cellNumber]++;
    }

    // setup Partial sums
    uint32_t sum = 0;
    for (auto & cell: cellParticleCount){
        sum += cell;
        cell = sum;
        //printf(", %lu", sum);
    }
    //printf("\n");
    //printf(" E");
    // fill the particleArray
    for (auto &particle: particleArray){
        cellParticleCount[particle.cellNumber]--;
        //printf(" E.1, x%u, y%u", particle.cellX, particle.cellY);
        //sleep_ms(1);
        cells[particle.getCellX()][particle.getCellY()].numberParticles++;
        cells[particle.getCellX()][particle.getCellY()].state = cellStateEnum::water;
        //printf(" E.2 cellParticleCount Size %u, reading cell %lu", cellParticleCount.size(), particle.cellNumber);
        //sleep_ms(1);
        particlePointers[cellParticleCount[particle.cellNumber]] = particle.particleId;
    }
    //printf(" F\n");
}


void fluidWindow::integrateParticles(){
    for(auto& particle : particleArray){
        if(currentState==enumBadgeState::displayname1){
            auto forceAtParticle = getGravityForceForParticle(particle);
            particle.vx += 60*forceAtParticle.first * timeStep;
            particle.vy += 60*forceAtParticle.second * timeStep;
        } else if(currentState==enumBadgeState::normalg){
            particle.vy+= 0.0039f * 20 * y * timeStep;
            particle.vx+= 0.0039f * 20 * x * timeStep;
        }
        float currX = particle.getX();
        float currY = particle.getY();
        int currCellX = particle.getCellX();
        int currCellY = particle.getCellY();
        float newX = currX + particle.vx*timeStep;
        float newY = currY + particle.vy*timeStep;
        float dtx = timeStep;
        float dty = timeStep;
        if(newX >= xsize || newX < 0){
            if(particle.vx>0){
                dtx = ((xsize-0.001) - currX)/particle.vx;
            } else{
                dtx = (0.001 - currX)/particle.vx;
            }
        }
        if(newY >= ysize || newY < 0){
            if(particle.vy>0){
                dty = ((ysize-0.001) - currY)/particle.vy;
            } else{
                dty = (0.001 - currY)/particle.vy;
            }
        }
        float dt = std::min(dtx,dty);
        newX = clamp<float>(currX + particle.vx*dt, 0, xsize - 0.001);
        newY = clamp<float>(currY + particle.vy*dt, 0, ysize - 0.001);
        uint8_t newCellX = floor(newX);
        uint8_t newCellY = floor(newY);
        uint8_t oldCellX = newCellX;
        uint8_t oldCellY = newCellY;
        // Error is in here somewhere.
        // If a particle is near a wall and would happen to go into it, it could have it's dt stopped by aton
        // that could slow it down a ton or even get it stuck.
        while(cells[newCellX][newCellY].isSolid()){
            dtx = 0;
            dty = 0;
            if(particle.vx>0){
                dtx = ((newCellX-0.005)-currX)/particle.vx;
            } else if(particle.vx<0){
                dtx = -((newCellX+0.995)-currX)/particle.vx;
            }
            if(particle.vy>0){
                dty = ((newCellY-0.005) - currY)/particle.vy;
            } else if(particle.vy<0){
                dty = -((newCellY+0.995) - currY)/particle.vy;
            }
            //printf("dtx, dty: %f, %f\n", dtx, dty);
            //printf("Particle velocity is %f, %f\n", particle.vx, particle.vy);
            //printf("Particle %lu Old coords %f, %f\n", particle.particleId, newX, newY);
            //printf("Current Cell X is %d, Current Cell Y is %d\n", currCellX, currCellY);
            
            dtx = dtx < 0 ? 0 : dtx;
            dty = dty < 0 ? 0 : dty;
            dt = std::min(dtx,dty);
            /*if(dt<timeStep/5){
                if(dtx<dty){
                    particle.vx = 0;
                } else {
                    particle.vy = 0;
                }
            }*/
            newX = clamp<float>(currX + particle.vx*dt, 0, xsize - 0.001);
            newY = clamp<float>(currY + particle.vy*dt, 0, ysize - 0.001);
            newCellX = floor(newX);
            newCellY = floor(newY);
            //printf("Particle %lu New coords %f, %f\n", particle.particleId, newX, newY);
            //sleep_ms(250);
        }
        if(particle.vx>0){
            if(cells[currCellX+1][currCellY].isSolid()){ particle.vx = -0.01; }
        } else {
            if(cells[currCellX-1][currCellY].isSolid()){ particle.vx = 0.01; }
        }
        if(particle.vy>0){
            if(cells[currCellX][currCellY+1].isSolid()){ particle.vy = -0.01; }
        } else {
            if(cells[currCellX][currCellY-1].isSolid()){ particle.vy = 0.01; }
        }
        particle.setCoordinates(newX,newY);
        if(currentState==enumBadgeState::displayname1) dampenParticleVelocity(particle, gravityField);
        if(particle.getCellX()!=oldCellX){
            particle.vx = 0;
        }
        if(particle.getCellY()!=oldCellY){
            particle.vy = 0;
        }
    }
    updateDataStructures();
}

void fluidWindow::handleParticleCollisions(){
    for(int iter = 0; iter < 5; iter++){
        for( auto &particle : particleArray){
            for( int i = particle.getCellX()-1; i <= particle.getCellX()+1; i++){
                for( int j = particle.getCellY()-1; j <= particle.getCellY()+1; j++){
                    //printf("Checking cell (%d, %d)\n", i, j);
                    auto particleStats = getParticleStats(i + xsize*j);
                    if(std::get<1>(particleStats)==0){
                        continue;
                    }
                    for(int particleOffset = std::get<0>(particleStats); particleOffset < (std::get<0>(particleStats)+std::get<1>(particleStats)); particleOffset++){
                        if(particleOffset >= numParticles){
                            printf("BAD PARTICLE ARRAY ACCESS: cell: %d, %d: array slot %d!\n", i, j, i + xsize*j);
                            printf("Number of particles: %u, Particle Offset %u\n", std::get<1>(particleStats), std::get<0>(particleStats));
                            printParticle(particle, "Is in a solid cell?");
                            sleep_ms(100);
                            continue;
                        }
                        auto& otherParticle = particleArray[particleOffset];
                        if (&particle == &otherParticle){
                            continue;
                        }
                        checkCollision(particle, otherParticle);
                    }
                }
            }
        //printParticle(particle, "after collisions");
        }
    }
}

std::tuple<uint16_t, uint16_t> fluidWindow::getParticleStats(uint32_t cellNumber){
    uint8_t pointerOffset = cellParticleCount[cellNumber];
    uint8_t numberOfParticles = cellParticleCount[cellNumber+1] - cellParticleCount[cellNumber];
    return std::make_tuple(pointerOffset, numberOfParticles);
}

void fluidWindow::checkCollision(fluidParticle& particle1, fluidParticle& particle2){
    //printf("Particle velocities: (%f, %f), (%f, %f)\n", particle1.vx, particle1.vy, particle2.vx, particle2.vy);
    
    float dx = particle1.getX() - particle2.getX();
    float dy = particle1.getY() - particle2.getY();
    float d2 = dx*dx + dy*dy;
    float r2 = particle1.diameter*particle2.diameter/4;
    if (d2 >= r2){
        return;
    }
    if(d2==0){
        // nudge particle 1 towards the center.
        float newX = particle1.getX();
        float newY = particle1.getY();
        if(particle1.getX()>xsize/2){
            newX-=particle1.diameter/1.41;
        } else {
            newX+=particle1.diameter/1.41;
        }
        if(particle1.getY()>ysize/2){
            newY-=particle1.diameter/1.41;
        } else {
            newY+=particle1.diameter/1.41;
        }
        particle1.setCoordinates(newX,newY);
    }

    //printf("Collision detected!\n");
    float d = sqrt(d2);
    float s = 0.5 * (particle1.diameter - d)/d;
    dx *= s;
    dy *= s;
    float newX1 = particle1.getX() + dx;
    float newY1 = particle1.getY() + dy;
    float newX2 = particle2.getX() - dx;
    float newY2 = particle2.getY() - dy;

    // Dampen particles that collide, 1% slower
    particle1.vx *= 0.999;
    particle1.vy *= 0.999;
    particle2.vx *= 0.999;
    particle2.vy *= 0.999;

    // If either particle would get pushed into the wall, don't half the disance since only one particle will be moved.
    if(cells[floor(newX1)][floor(newY1)].isSolid() || cells[floor(newX2)][floor(newY2)].isSolid()){
        //printf("Doubling distance!\n");
        newX1 = particle1.getX() + 2*dx;
        newY1 = particle1.getY() + 2*dy;
        newX2 = particle2.getX() - 2*dx;
        newY2 = particle2.getY() - 2*dy;
    }
    //printParticle(particle1,"Particle1");
    //printParticle(particle2,"Particle2");
    //printf("Two particles being moved to %f, %f and %f, %f\n", newX1, newY1, newX2, newY2);
    if(!cells[floor(newX1)][floor(newY1)].isSolid()){
        particle1.setCoordinates(newX1, newY1);
    }
    if(!cells[floor(newX2)][floor(newY2)].isSolid()){
        particle2.setCoordinates(newX2, newY2);
    }
}

void fluidWindow::simulateParticles(){
    integrateParticles();
    //printf("After integration:\n");
    //printParticles(50);
    handleParticleCollisions();
    //printf("After Collisions:\n");
}

void fluidWindow::makeIncompressible(uint8_t iterations) {
    for (uint8_t i = 0; i < iterations; i++) {
        for (auto &column : cells) {
            for (auto &cell : column) {

                if (cell.state != cellStateEnum::water) {
                    continue;
                }
                fluidCell& rightCell = right(cell);
                fluidCell& downCell = down(cell);
                fluidCell& leftCell = left(cell);
                fluidCell& upCell = up(cell);
                
                // Calculate divergence, positive is outflow.
                float divergence = (-cell.horizontalFlow*left(cell).flowAllowed 
                                  + right(cell).horizontalFlow*right(cell).flowAllowed  
                                  - cell.verticalFlow*up(cell).flowAllowed 
                                  + down(cell).verticalFlow*down(cell).flowAllowed);
                
                int solidMultiplier = left(cell).flowAllowed + right(cell).flowAllowed + 
                                      up(cell).flowAllowed + down(cell).flowAllowed;

                if(solidMultiplier == 0){
                    continue;
                }

                float compression = cell.numberParticles - particleDensity;
                compression = compression > 0 ? compression : 0;
                divergence = 2*(divergence) - 1.5f*compression;    

                cell.horizontalFlow += divergence * leftCell.flowAllowed / solidMultiplier;
                rightCell.horizontalFlow -= divergence * rightCell.flowAllowed / solidMultiplier;
                cell.verticalFlow += divergence * upCell.flowAllowed / solidMultiplier;
                downCell.verticalFlow -= divergence * downCell.flowAllowed / solidMultiplier;  
            }
        }
    }
}


void fluidWindow::toGrid(){
    // Reset grid values
    for (auto &column : cells) {
        for (auto &cell : column) {            
            if (cell.state == cellStateEnum::water) {
                cell.state = cellStateEnum::air;
            }
            cell.horizontalFlow = 0.0f;
            cell.verticalFlow = 0.0f;
            // Reset weights
            cell.horizontalWeight = 0.0f;
            cell.verticalWeight = 0.0f;
        }
    }

    for(auto& particle : particleArray){
        // Horizontal Flow
        uint8_t rootCellX = particle.getCellX();
        uint8_t rootCellY = (particle.getY()-particle.getCellY()) > 0.5 ? particle.getCellY() : particle.getCellY()-1;
        fluidCell& topLeftHori = cells[rootCellX][rootCellY];
        fluidCell& topRightHori = right(topLeftHori);
        fluidCell& bottomLeftHori = down(topLeftHori);
        fluidCell& bottomRightHori = right(bottomLeftHori);
        float dx = particle.getX() - (topLeftHori.x);
        float sx = 1-dx;
        float dy = particle.getY() - (topLeftHori.y + 0.5);
        float sy = 1-dy;
        float tlWeight = sx*sy;
        float trWeight = dx*sy;
        float blWeight = sx*dy;
        float brWeight = dx*dy;

        topLeftHori.horizontalFlow   += tlWeight*particle.vx;
        topLeftHori.horizontalWeight += tlWeight;
        topRightHori.horizontalFlow   += trWeight*particle.vx;
        topRightHori.horizontalWeight += trWeight;
        bottomLeftHori.horizontalFlow   += blWeight*particle.vx;
        bottomLeftHori.horizontalWeight += blWeight;
        bottomRightHori.horizontalFlow   += brWeight*particle.vx;
        bottomRightHori.horizontalWeight += brWeight;

        // Vertical Flow

        rootCellX = (particle.getX()-particle.getCellX()) > 0.5 ? particle.getCellX() : particle.getCellX()-1;
        rootCellY = particle.getCellY();
        fluidCell& topLeftVert = cells[rootCellX][rootCellY];
        fluidCell& topRightVert = right(topLeftVert);
        fluidCell& bottomLeftVert = down(topLeftVert);
        fluidCell& bottomRightVert = right(bottomLeftVert);
        dx = particle.getX() - (topLeftVert.x + 0.5);
        sx = 1-dx;
        dy = particle.getY() - (topLeftVert.y);
        sy = 1-dy;
        tlWeight = sx*sy;
        trWeight = dx*sy;
        blWeight = sx*dy;
        brWeight = dx*dy;

        topLeftVert.verticalFlow     += tlWeight*particle.vy;
        topLeftVert.verticalWeight   += tlWeight;
        topRightVert.verticalFlow     += trWeight*particle.vy;
        topRightVert.verticalWeight   += trWeight;
        bottomLeftVert.verticalFlow     += blWeight*particle.vy;
        bottomLeftVert.verticalWeight   += blWeight;
        bottomRightVert.verticalFlow     += brWeight*particle.vy;
        bottomRightVert.verticalWeight   += brWeight;
        if(cells[particle.getCellX()][particle.getCellY()].isAir()){
            cells[particle.getCellX()][particle.getCellY()].state = cellStateEnum::water;
        }
    }

    // Restore solid cells
    for (auto &column : cells) {
        for (auto &cell : column) {
            if (cell.horizontalWeight > 0.0f)
                cell.horizontalFlow /= cell.horizontalWeight;

            if (cell.verticalWeight > 0.0f)
                cell.verticalFlow /= cell.verticalWeight;

            if (cell.state == cellStateEnum::solid) {
                cell.horizontalFlow = 0;
                cell.verticalFlow = 0;
                right(cell).horizontalFlow = 0;
                down(cell).verticalFlow = 0;
            }
            cell.prevHorizontalFlow = cell.horizontalFlow;
            cell.prevVerticalFlow = cell.verticalFlow;
        }
    }
}

void fluidWindow::fromGrid(float ratio){
    for(auto& particle : particleArray){
        // Horizontal Flow
        uint8_t rootCellX = particle.getCellX();
        uint8_t rootCellY = (particle.getY()-particle.getCellY()) > 0.5 ? particle.getCellY() : particle.getCellY()-1;
        fluidCell& topLeftHori = cells[rootCellX][rootCellY];
        fluidCell& topRightHori = right(topLeftHori);
        fluidCell& bottomLeftHori = down(topLeftHori);
        fluidCell& bottomRightHori = right(bottomLeftHori);
        float dx = particle.getX() - (topLeftHori.x);
        float sx = 1-dx;
        float dy = particle.getY() - (topLeftHori.y + 0.5);
        float sy = 1-dy;
        float tlWeight = sx*sy;
        float trWeight = dx*sy;
        float blWeight = sx*dy;
        float brWeight = dx*dy;
        float tlValid = !topLeftHori.isSolid() ? 1 : 0;
        float trValid = !topRightHori.isSolid() ? 1 : 0;
        float blValid = !bottomLeftHori.isSolid() ? 1 : 0;
        float brValid = !bottomRightHori.isSolid() ? 1 : 0;
        float validWeight = tlValid*tlWeight + trValid*trWeight + blValid*blWeight + brValid*brWeight;
        if(validWeight>0){
            float picH  = (tlValid*tlWeight*topLeftHori.horizontalFlow + 
                           trValid*trWeight*topRightHori.horizontalFlow +
                           blValid*blWeight*bottomLeftHori.horizontalFlow + 
                           brValid*brWeight*bottomRightHori.horizontalFlow)/validWeight;
            float corrH = (tlValid*tlWeight*(topLeftHori.horizontalFlow-topLeftHori.prevHorizontalFlow) +
                           trValid*trWeight*(topRightHori.horizontalFlow-topRightHori.prevHorizontalFlow) +
                           blValid*blWeight*(bottomLeftHori.horizontalFlow-bottomLeftHori.prevHorizontalFlow) +
                           brValid*brWeight*(bottomRightHori.horizontalFlow-bottomRightHori.prevHorizontalFlow))/validWeight;
            float flipH = particle.vx + corrH;
            particle.vx = flipH * ratio + picH * (1.0-ratio);
        }
        // Vertical Flow
        rootCellX = (particle.getX()-particle.getCellX()) > 0.5 ? particle.getCellX() : particle.getCellX()-1;
        rootCellY = particle.getCellY();
        fluidCell& topLeftVert = cells[rootCellX][rootCellY];
        fluidCell& topRightVert = right(topLeftVert);
        fluidCell& bottomLeftVert = down(topLeftVert);
        fluidCell& bottomRightVert = right(bottomLeftVert);
        dx = particle.getX() - (topLeftVert.x + 0.5);
        sx = 1-dx;
        dy = particle.getY() - (topLeftVert.y);
        sy = 1-dy;
        tlWeight = sx*sy;
        trWeight = dx*sy;
        blWeight = sx*dy;
        brWeight = dx*dy;
        tlValid = !topLeftVert.isSolid() ? 1 : 0;
        trValid = !topRightVert.isSolid() ? 1 : 0;
        blValid = !bottomLeftVert.isSolid() ? 1 : 0;
        brValid = !bottomRightVert.isSolid() ? 1 : 0;
        validWeight = tlValid*tlWeight + trValid*trWeight + blValid*blWeight + brValid*brWeight;
        if(validWeight>0){
            float picV  = (tlValid*tlWeight*topLeftVert.verticalFlow + 
                           trValid*trWeight*topRightVert.verticalFlow +
                           blValid*blWeight*bottomLeftVert.verticalFlow + 
                           brValid*brWeight*bottomRightVert.verticalFlow)/validWeight;
            float corrV = (tlValid*tlWeight*(topLeftVert.verticalFlow-topLeftVert.prevVerticalFlow) + 
                           trValid*trWeight*(topRightVert.verticalFlow-topRightVert.prevVerticalFlow) +
                           blValid*blWeight*(bottomLeftVert.verticalFlow-bottomLeftVert.prevVerticalFlow) + 
                           brValid*brWeight*(bottomRightVert.verticalFlow-bottomRightVert.prevVerticalFlow))/validWeight;
            float flipV = particle.vy + corrV;
            particle.vy = flipV * ratio + picV * (1.0-ratio);
        }
    }
}






void fluidWindow::printParticles(int iter =  99999){
    for(auto particle: particleArray){
        if(iter--<0){break;}
        printf("Particle id%u at (%f, %f) cell(%d, %d) with velocity (%f, %f)\n", particle.particleId, particle.getX(), particle.getY(), particle.getCellX(), particle.getCellY(), particle.vx, particle.vy);
    }
}

//int testLed = 1;
void fluidWindow::stepSim(){
    //printf("Loop!\n");
    simulateParticles();
    //myWindow.printParticles();
    //printf("Simulated!\n");
    toGrid();
    //myWindow.printParticles();
    //printf("Particles to cells!\n");
    makeIncompressible(40);
    //myWindow.printParticles();
    //printf("Incompressible!\n");   
    fromGrid(0.9);
    //myWindow.printParticles();
    //printf("Cells to particles!\n");    
    //sleep_ms(10);
    print();
    //ledBuffer2[testLed] = 0;
    //testLed++;
    //if(testLed>192){
    //    testLed = 1;
    //}
    //ledBuffer2[testLed] = 200;
    loopNumber++;

    if(loopNumber == 1200){
        //gpio_put(25, 1);
        currentState = enumBadgeState::zerog;
    }  else if (loopNumber == 1600){
        currentState = enumBadgeState::displayname1;
    } else if (loopNumber == 2400){
        //gpio_put(25, 0);
        currentState = enumBadgeState::normalg;
        loopNumber = 0;
    }

}

fluidWindow myWindow;

void periodic_task_sim() {
    absolute_time_t next_time = make_timeout_time_ms(20);
    while (true) {
        //printf("In sim task!\n");
        sleep_until(next_time);
        myWindow.stepSim();
        next_time = delayed_by_ms(next_time, 100/6);
    }
}

void periodic_task_io() {
    int16_t oldX, oldY, oldZ;
    oldX = 0;
    oldY = 0;
    oldZ = 0;
    absolute_time_t next_time = make_timeout_time_ms(20);
    while (true) {
        //printf("In IO task! X is %d.\n", x);
        if(x>0){
            gpio_put(25, 1);
        } else {
            gpio_put(25, 0);
        }
        sleep_until(next_time);
        oldX = x;
        oldY = y;
        oldZ = z;
        i2c_read_accel(x,y,z);
        //variance = variance * 0.997 + 0.003*(abs(x-oldX) + abs(y-oldY) + abs(z-oldZ));
        //printf("x:%d oldx:%d y:%d oldy:%d z:%d oldz:%d \n", x, oldX, y, oldY, z, oldZ);
        //printf("variance: %f", variance);
        //if(variance < 250){
        //    gpio_put(21, 0);
        //} else {
        //    gpio_put(21, 1);
        //}

        set_all_brightness();
        next_time = delayed_by_ms(next_time, 100/6);
    }
}

int main() {
    set_sys_clock_khz(250000, true);

    stdio_init_all(); // Initialize standard IO


    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SCL_PIN);

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    gpio_init(21);
    gpio_set_dir(21, GPIO_OUT);
    gpio_put(21, 1);

    ledBuffer1[0]=0;
    ledBuffer2[0]=0;


    is31fl3733_init();
    sleep_ms(10);
    printf("Startup\n");
    sleep_ms(10);
    myWindow.init();
    printf("Init!\n");
    //myWindow.simulateParticles();
    set_all_brightness();

    multicore_launch_core1(periodic_task_sim);
    periodic_task_io();
}
