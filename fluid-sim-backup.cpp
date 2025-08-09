#include "pico/stdlib.h"
#include "fluid-sim.h"
#include <cstdio> // Include the C standard IO functions
#include <math.h>
#include <random>

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

void fluidWindow::print(){
    printf("\n\n\n");
    for (size_t j = 0; j < ysize; j++){
        for (size_t i = 0; i < xsize; i++){
            switch (cells[i][j].state){
                case cellStateEnum::solid:
                    printf("##");
                    break;
                case cellStateEnum::air:
                    printf("  ");
                    break;
                case cellStateEnum::water:
                    printf("~~");
                    break;
            }
        }
        printf("\n");
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
    // Setup the walls
    for (size_t i = 0; i < xsize; i++){
        cells[i][0].state = cellStateEnum::solid;
        cells[i][0].flowAllowed = 0;
        cells[i][ysize-1].state = cellStateEnum::solid;
        cells[i][ysize-1].flowAllowed = 0;
    }
    for (size_t j = 0; j < ysize; j++){
        cells[0][j].state = cellStateEnum::solid;
        cells[0][j].flowAllowed = 0;
        cells[xsize-1][j].state = cellStateEnum::solid;
        cells[xsize-1][j].flowAllowed = 0;
    }
    for( size_t i = 0; i < xsize; i++){
        for( size_t j = 0; j < ysize; j++){
            cells[i][j].x = i;
            cells[i][j].y = j;
        }
    }
    // Setup the particles
    uint32_t particleId = 0;
    printf("Init coord set\n;");
    for(auto& particle: particleArray){
        particle.setCoordinates(getRandomFloat(1,xsize-1.001),getRandomFloat(1,ysize-1.001));
        particle.vx = 0;
        particle.vy = 0;
        particle.particleId = particleId;
        particleId++;
    }
}

void fluidWindow::updateDataStructures(){
    //printf("A");
    for(auto &colmn: cells){
        for(auto& cell: colmn){
            cell.numberParticles = 0;
        }
    }

    //printf(" B");
    // Clear the cellParticleCount array
    for (auto &cell: cellParticleCount){
        cell = 0;
    }
    //printf(" C");
    // Update the particleCell array
    for (auto &particle: particleArray){
        cellParticleCount[particle.cellNumber]++;
    }
    //printf(" D");  
    // setup Partial sums
    uint32_t sum = 0;
    for (auto & cell: cellParticleCount){
        sum += cell;
        cell = sum;
    }
    //printf(" E");
    // fill the particleArray
    for (auto &particle: particleArray){
        cellParticleCount[particle.cellNumber]--;
        //printf(" E.1, x%u, y%u", particle.cellX, particle.cellY);
        //sleep_ms(1);
        cells[particle.getCellX()][particle.getCellY()].numberParticles++;
        //printf(" E.2 cellParticleCount Size %u, reading cell %lu", cellParticleCount.size(), particle.cellNumber);
        //sleep_ms(1);
        particlePointers[cellParticleCount[particle.cellNumber]] = particle.particleId;
    }
    //printf(" F\n");
}

void fluidWindow::simulateParticles(){
    //printf("Starting Particle Simulation!\n");
    //printf("Checking first particle access...\n");
    //printf("Translating particles!\n");
    for( auto &particle : particleArray){
        // Apply acceleration
        //printf("Particle velocity before, after, before solid calcs: %f,", particle.vy);
        particle.vy += gravity*timeStep;
        //printf(" %f", particle.vy);

        // Translate particle.
        float curX = particle.getX();
        float curY = particle.getY();
        float newX = curX + particle.vx*timeStep;
        float newY = curY + particle.vy*timeStep;

        // Check for out of bounds particles.
        float dtx = 0;
        float dty = 0;
        if(newX<0 || newX>=xsize){
            if(particle.vx>0){
                dtx = (xsize-curX)/particle.vx;
            } else {
                dtx = (0-curX)/particle.vx;
            }
        }
        if(newY<0 || newY>=ysize){
            if(particle.vy>0){
                dty = (ysize-curY)/particle.vy;
            } else {
                dty = (0-curY)/particle.vy;
            }
        }
        if(dtx==0 && dty == 0){
            particle.setCoordinates(newX, newY);
        } else {
            printParticle(particle, "Particle will be out of bounds");
            printf("dtx: %f, dty: %f\n", dtx, dty);
            if(dtx == 0){
                dtx = dty;
            } else if (dty == 0){
                // do nothing, assume dtx is correct value
            } else {
                dtx = std::min(dtx, dty);
            }
            newX = curX + particle.vx*(dtx-0.0001);
            newY = curY + particle.vy*(dtx-0.0001);
            particle.setCoordinates(newX, newY);

            printParticle(particle, "Particle was out of bounds");
        } 


        //printf(" %f\n", particle.vy);
        //printParticle(particle, "End of particle translate");

    }
    //printf("Particles translated, handling solid cell\n");
    handleSolidCells();        
    //printf("Particles Moved!\n");
    updateDataStructures();
    //printf("Data Structures Updated!\n");

    // check for collisions:
    //printf("Handling collisions\n");
    for(int iter = 0; iter < 2; iter++){
        for( auto &particle : particleArray){
            for( int i = particle.getCellX()-1; i <= particle.getCellX()+1; i++){
                for( int j = particle.getCellY()-1; j <= particle.getCellY()+1; j++){
                    //printf("Checking cell (%d, %d)\n", i, j);
                    auto particleStats = getParticleStats(getCellNumberFromCords(i, j));
                    if(std::get<1>(particleStats)==0){
                        continue;
                    }
                    for(int particleOffset = std::get<0>(particleStats); particleOffset < std::get<0>(particleStats)+std::get<1>(particleStats); particleOffset++){
                        if(particleOffset >= numParticles){
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
    //printf("Solid cells again!\n");
    handleSolidCells();

}

std::tuple<uint8_t, uint8_t> fluidWindow::getParticleStats(uint32_t cellNumber){
    uint8_t pointerOffset = cellParticleCount[cellNumber];
    uint8_t numberOfParticles = cellParticleCount[cellNumber+1] - cellParticleCount[cellNumber];
    return std::make_tuple(pointerOffset, numberOfParticles);
}

void fluidWindow::checkCollision(fluidParticle& particle1, fluidParticle& particle2){
    //printf("Particle velocities: (%f, %f), (%f, %f)\n", particle1.vx, particle1.vy, particle2.vx, particle2.vy);
    
    float dx = particle1.getX() - particle2.getX();
    float dy = particle1.getY() - particle2.getY();
    float d2 = dx*dx + dy*dy;
    float nx = 0;
    float ny = 0;
    float r2 = particle1.diameter*particle1.diameter/4;
    if (d2 > r2){
        return;
    }
    int maxIter = 10;
    while(d2 < r2){
        if(maxIter<=0){
            printf("STUCK IN COLLISION\n");
            printParticle(particle1,"");
            printParticle(particle2,"");
            if(maxIter<0){break;}
            maxIter--;
        }
        //printf("Collision detected!\n");
        float d = sqrt(d2);
        if(d==0){
            // particles directly on each other, nudge particle 1.
            if(particle1.getX()<(xsize/2)){
                particle1.setCoordinates(particle1.getX()+particle1.diameter/10,particle1.getY());
            } else {
                particle1.setCoordinates(particle1.getX()-particle1.diameter/10,particle1.getY());
            }
            if(particle1.getY()<(ysize/2)){
                particle1.setCoordinates(particle1.getX(),particle1.getY()+particle1.diameter/10);
            } else {
                particle1.setCoordinates(particle1.getX(),particle1.getY()-particle1.diameter/10);
            }
            dx = particle1.getX() - particle2.getX();
            dy = particle1.getY() - particle2.getY();
            d2 = dx*dx + dy*dy;
            continue;
        }
        nx = dx/d;
        ny = dy/d;
        float newX1 = particle1.getX() + nx/2;
        float newY1 = particle1.getY() + ny/2;
        float newX2 = particle2.getX() - nx/2;
        float newY2 = particle2.getY() - ny/2;

        // If either particle would get pushed into the wall, don't half the disance since only one particle will be moved.
        if(cells[floor(newX1)][particle1.getCellY()].state == cellStateEnum::solid ||
            cells[floor(newX2)][particle2.getCellY()].state == cellStateEnum::solid){
            newX1 = particle1.getX() + nx;
            newX2 = particle2.getX() - nx;
        }
        if(cells[particle1.getCellX()][floor(newY1)].state == cellStateEnum::solid ||
            cells[particle2.getCellX()][floor(newY1)].state == cellStateEnum::solid){
            newY1 = particle1.getY() + ny;
            newY2 = particle2.getY() - ny;
        }
        
        if((cells[floor(newX1)][floor(newY1)].state != cellStateEnum::solid) ){
            particle1.setCoordinates(newX1, newY1);
        }else if((cells[floor(newX1)][particle1.getCellY()].state != cellStateEnum::solid)){
            particle1.setCoordinates(newX1, particle1.getY());
        }else if((cells[particle1.getCellX()][floor(newY1)].state != cellStateEnum::solid)){
            particle1.setCoordinates(particle1.getX(), newY1);
        }
        if((cells[floor(newX2)][floor(newY2)].state != cellStateEnum::solid) ){
            particle2.setCoordinates(newX2, newY2);
        }else if((cells[floor(newX2)][particle2.getCellY()].state != cellStateEnum::solid)){
            particle2.setCoordinates(newX2, particle2.getY());
        }else if((cells[particle1.getCellX()][floor(newY1)].state != cellStateEnum::solid)){
            particle2.setCoordinates(particle2.getX(), newY2);
        }

        dx = particle1.getX() - particle2.getX();
        dy = particle1.getY() - particle2.getY();
        d2 = dx*dx + dy*dy;
        d = sqrt(d2);
        nx = dx/d;
        ny = dy/d;

        //printParticle(particle1, "Particle 1 after collision");
        //printParticle(particle2, "Particle 2 after collision");
    }
    // Compute relative velocity
    float rvx = particle1.vx - particle2.vx;
    float rvy = particle1.vy - particle2.vy;

    //printf("Relative velocity: (%f, %f)\n", rvx, rvy);

    // Compute velocity along the normal
    float normalVelocity = rvx * nx + rvy * ny;

    // If particles are moving away from each other, no need to reflect
    if (normalVelocity > 0) {
        return;
    }
    //printf("Momentum before is %f,%f.", particle1.vx+particle2.vx,particle1.vy+particle2.vy);

    // Reflect velocities
    //float impulse = - normalVelocity / 2; // Divide by 2 (equal mass assumption)
    //printf("Normal velocity: %f, impulse: %f\n", normalVelocity, impulse);
    //printf("Reflecting velocities, before, after: (%f, %f),", particle1.vx, particle1.vy);
    //particle1.vx = (particle1.vx - impulse * nx);
    //particle1.vy = (particle1.vx - impulse * ny);
    //particle2.vx = (particle2.vx + impulse * nx);
    //particle2.vy = (particle2.vx + impulse * ny);
    //printf("after is %f,%f.\n", particle1.vx+particle2.vx,particle1.vy+particle2.vy);
    //printf(" (%f, %f)\n", particle1.vx, particle1.vy);
}

void fluidWindow::handleSolidCells(){
    for(auto& particle: particleArray){
        // Handle particles that are in solid cells
        //printf("Starting particle %lu\n", particle.particleId);
        uint8_t oldCellX = particle.getCellX();
        uint8_t oldCellY = particle.getCellY();
        float dtx = 0;
        float dty = 0;
        //sleep_ms(10);
        int numLoops = 10;
        while(cells[particle.getCellX()][particle.getCellY()].state == cellStateEnum::solid){
            if(numLoops<=0){
                printf("STUCK IN SOILD CELL LOOP\n");
                printParticle(particle,"");
                if(numLoops<0){break;}
            }
            numLoops--;
            //printf("Particle cell: (%d, %d)\n", particle.cellX, particle.cellY);
            oldCellX = particle.getCellX();
            oldCellY = particle.getCellY();
            printParticle(particle, "Particle in solid cell");
            //sleep_ms(100);
            if(particle.vx>0){
                dtx = (particle.getCellX()-particle.getX())/particle.vx;
            } else {
                dtx = ((particle.getCellX() + 1) - particle.getX())/particle.vx;
            }
            if(particle.vy>0){
                dty = (particle.getCellY() - particle.getY())/particle.vy;
            } else {
                dty = ((particle.getCellY() +1)- particle.getY())/particle.vy;
            }
            //printf("Solid cell dtx: %f, dty: %f\n", dtx, dty);
            dtx = std::isnan(dtx) ? -100 : std::isinf(dtx)? -100 : dtx;
            dty = std::isnan(dty) ? -100 : std::isinf(dty)? -100 : dty;
            dtx = std::max(dtx, dty)-0.0001;
            float newX = particle.getX() + particle.vx*dtx;
            float newY = particle.getY() + particle.vy*dtx;
            particle.setCoordinates(newX,newY);
            printParticle(particle, "Particle moved out of solid cell");
        }
        if(oldCellX != particle.getCellX()){
            particle.vx = 0;
        }  
        if(oldCellY != particle.getCellY()){
            particle.vy = 0;
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
        uint8_t rootCellX = (particle.getX()-particle.getCellX()) > 0.5 ? particle.getCellX() : particle.getCellX()-1;
        uint8_t rootCellY = (particle.getY()-particle.getCellY()) > 0.5 ? particle.getCellY() : particle.getCellY()-1;
        fluidCell& topLeft = cells[rootCellX][rootCellY];
        fluidCell& topRight = right(topLeft);
        fluidCell& bottomLeft = down(topLeft);
        fluidCell& bottomRight = right(bottomLeft);
        float dx = particle.getX() - (topLeft.x + 0.5);
        float sx = 1-dx;
        float dy = particle.getY() - (topLeft.y + 0.5);
        float sy = 1-dy;
        float tlWeight = dx*dy;
        float trWeight = sy*dy;
        float blWeight = dx*sy;
        float brWeight = sx*sy;
        topLeft.horizontalFlow   += tlWeight*particle.vx;
        topLeft.horizontalWeight += tlWeight;
        topLeft.verticalFlow     += tlWeight*particle.vy;
        topLeft.verticalWeight   += tlWeight;

        topRight.horizontalFlow   += trWeight*particle.vx;
        topRight.horizontalWeight += trWeight;
        topRight.verticalFlow     += trWeight*particle.vy;
        topRight.verticalWeight   += trWeight;

        bottomLeft.horizontalFlow   += blWeight*particle.vx;
        bottomLeft.horizontalWeight += blWeight;
        bottomLeft.verticalFlow     += blWeight*particle.vy;
        bottomLeft.verticalWeight   += blWeight;

        bottomRight.horizontalFlow   += brWeight*particle.vx;
        bottomRight.horizontalWeight += brWeight;
        bottomRight.verticalFlow     += brWeight*particle.vy;
        bottomRight.verticalWeight   += brWeight;
        if(!cells[particle.getCellX()][particle.getCellY()].isSolid()){
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
        uint8_t rootCellX = (particle.getX()-particle.getCellX()) > 0.5 ? particle.getCellX() : particle.getCellX()-1;
        uint8_t rootCellY = (particle.getY()-particle.getCellY()) > 0.5 ? particle.getCellY() : particle.getCellY()-1;
        fluidCell& topLeft = cells[rootCellX][rootCellY];
        fluidCell& topRight = right(topLeft);
        fluidCell& bottomLeft = down(topLeft);
        fluidCell& bottomRight = right(bottomLeft);
        float dx = particle.getX() - (topLeft.x + 0.5);
        float sx = 1-dx;
        float dy = particle.getY() - (topLeft.y + 0.5);
        float sy = 1-dy;
        float tlWeight = dx*dy;
        float trWeight = sy*dy;
        float blWeight = dx*sy;
        float brWeight = sx*sy;
        printf("Particle id %lu at %f, %f: Dx, Dy: %f, %f\n", particle.particleId, particle.getX(), particle.getY(), dx, dy);
        float tlValid = !topLeft.isSolid() ? 1 : 0;
        float trValid = !topRight.isSolid() ? 1 : 0;
        float blValid = !bottomLeft.isSolid() ? 1 : 0;
        float brValid = !bottomRight.isSolid() ? 1 : 0;
        float validWeight = tlValid*tlWeight + trValid*trWeight + blValid*blWeight + brValid*brWeight;
        if(validWeight>0){
            float picH  = (tlValid*tlWeight*topLeft.horizontalFlow + trValid*trWeight*topRight.horizontalFlow +
                           blValid*blWeight*bottomLeft.horizontalFlow + brValid*brWeight*bottomRight.horizontalFlow)/validWeight;
            float picV  = (tlValid*tlWeight*topLeft.verticalFlow + trValid*trWeight*topRight.verticalFlow +
                           blValid*blWeight*bottomLeft.verticalFlow + brValid*brWeight*bottomRight.verticalFlow)/validWeight;
            float corrH = (tlValid*tlWeight*(topLeft.horizontalFlow-topLeft.prevHorizontalFlow) + trValid*trWeight*(topRight.horizontalFlow-topRight.prevHorizontalFlow) +
                           blValid*blWeight*(bottomLeft.horizontalFlow-bottomLeft.prevHorizontalFlow) + brValid*brWeight*(bottomRight.horizontalFlow-bottomRight.prevHorizontalFlow))/validWeight;
            float corrV = (tlValid*tlWeight*(topLeft.verticalFlow-topLeft.prevVerticalFlow) + trValid*trWeight*(topRight.verticalFlow-topRight.prevVerticalFlow) +
                           blValid*blWeight*(bottomLeft.verticalFlow-bottomLeft.prevVerticalFlow) + brValid*brWeight*(bottomRight.verticalFlow-bottomRight.prevVerticalFlow))/validWeight;
            float flipH = particle.vx + corrH;
            float flipV = particle.vy + corrV;
            printf("Particle %lu at (%u,%u) pic and flip: %f, %f, %f, %f\n", particle.particleId, particle.getCellX(),particle.getCellY(),picH, picV, flipH, flipV);
            particle.vx = flipH * ratio + picH * (1.0-ratio);
            particle.vy = flipV * ratio + picV * (1.0-ratio);
        }


    }
}


uint32_t fluidWindow::getCellNumberFromParticle(float x, float y){
    return floor(x) + xsize*floor(y);
}

uint32_t fluidWindow::getCellNumberFromCords(uint8_t x, uint8_t y){
    return x + xsize*y;
}

void fluidWindow::particlesToCells() {
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

    // Transfer particle velocities to grid using bilinear interpolation
    for (auto &particle : particleArray) {
        fluidCell& currentCell = cells[particle.getCellX()][particle.getCellY()];
        fluidCell& bottomCell  = cells[particle.getCellX()][particle.getCellY()+1];
        fluidCell& rightCell   = cells[particle.getCellX()+1][particle.getCellY()];

        float dx = particle.getX() - particle.getCellX();
        float dy = particle.getY() - particle.getCellY();

        float sx = 1.0f - dx;
        float sy = 1.0f - dy;

        //printf("Particle id %lu dx sx dy sy: %f, %f, %f, %f\n", particle.particleId, dx, sx, dy, sy);

        currentCell.horizontalFlow += particle.vx * sx;
        rightCell.horizontalFlow += particle.vx * dx;

        currentCell.verticalFlow += particle.vy * sy;
        bottomCell.verticalFlow += particle.vy * dy;

        currentCell.horizontalWeight += sx;
        rightCell.horizontalWeight += dx;

        currentCell.verticalWeight += sy;
        bottomCell.verticalWeight += dy;

        currentCell.state = cellStateEnum::water;
        //printf("Cell (%d, %d) horizontal flow: %f, vertical flow: %f\n", currentCell.x, currentCell.y, currentCell.horizontalFlow, currentCell.verticalFlow);
    }

    // Normalize the grid velocities
    for (auto &column : cells) {
        for (auto &cell : column) {
            if (cell.horizontalWeight > 0.0f)
                cell.horizontalFlow /= cell.horizontalWeight;

            if (cell.verticalWeight > 0.0f)
                cell.verticalFlow /= cell.verticalWeight;
        }
    }

    // Restore solid cells
    for (auto &column : cells) {
        for (auto &cell : column) {
            cell.prevHorizontalFlow = cell.horizontalFlow;
            cell.prevVerticalFlow = cell.verticalFlow;
            if (cell.state == cellStateEnum::solid) {
                cell.horizontalFlow = 0;
                cell.verticalFlow = 0;
                right(cell).horizontalFlow = 0;
                down(cell).verticalFlow = 0;
            }
            printf("After solid cells (%d, %d) horizontal flow: %f, vertical flow: %f\n", cell.x, cell.y, cell.horizontalFlow, cell.verticalFlow);
        }
    }
}


void fluidWindow::makeIncompressible(uint8_t iterations) {
    for (uint8_t i = 0; i < iterations; i++) {
        for (auto &column : cells) {
            for (auto &cell : column) {
                //printf("Cell (%d, %d) horizontal flow: %f, vertical flow: %f\n", cell.x, cell.y, cell.horizontalFlow, cell.verticalFlow);
                //sleep_ms(10);
                if (cell.state != cellStateEnum::water) {
                    continue;
                }
                
                // Calculate divergence, positive is outflow.
                float divergence = (-cell.horizontalFlow*left(cell).flowAllowed 
                                  + right(cell).horizontalFlow*right(cell).flowAllowed  
                                  - cell.verticalFlow*up(cell).flowAllowed 
                                  + down(cell).verticalFlow*down(cell).flowAllowed);
                
                int solidMultiplier = left(cell).flowAllowed + right(cell).flowAllowed + 
                                      up(cell).flowAllowed + down(cell).flowAllowed;

                if(solidMultiplier == 0){
                    // Should never get here, but for div zero safety.
                    continue;
                }
                //printf("\nInitial divergence and solid mult: %f, %u\n", divergence, solidMultiplier);

                // hyperrelaxation
                divergence = 1.9*(divergence - cell.numberParticles-particleDensity);    
                //printf("Particle density, numParticles: %f, %ld\n", particleDensity, cell.numberParticles);
                /*if(solidMultiplier==2){
                    // Were in a corner, flow in one side must equal flow on the other, set all valid sides to divergence.
                    cell.horizontalFlow = divergence * left(cell).flowAllowed / solidMultiplier;
                    right(cell).horizontalFlow = divergence * right(cell).flowAllowed / solidMultiplier;
                    cell.verticalFlow = divergence * up(cell).flowAllowed / solidMultiplier;
                    down(cell).verticalFlow = divergence * down(cell).flowAllowed / solidMultiplier;
                    continue;
                }*/

                // Distribute the divergence adjustment
                cell.horizontalFlow += divergence * left(cell).flowAllowed / solidMultiplier;
                right(cell).horizontalFlow -= divergence * right(cell).flowAllowed / solidMultiplier;
                cell.verticalFlow += divergence * up(cell).flowAllowed / solidMultiplier;
                down(cell).verticalFlow -= divergence * down(cell).flowAllowed / solidMultiplier;
                if(i==iterations-1){
                    printf("\nCell cords %u, %u, solid mult: %d\n", cell.x, cell.y, solidMultiplier);
                    printf("Num particles is %lu, divergence is %f\n", cell.numberParticles, divergence);
                    printf("flow hori, vert: %f, %f\n", cell.horizontalFlow, cell.verticalFlow);

                }   
            }
        }
    }
}


void fluidWindow::cellsToParticles(float ratio) {
    for (auto &particle : particleArray) {
        fluidCell& currentCell = cells[particle.getCellX()][particle.getCellY()];
        fluidCell& topCell  = cells[particle.getCellX()][particle.getCellY()-1];
        fluidCell& bottomCell  = cells[particle.getCellX()][particle.getCellY()+1];
        fluidCell& rightCell   = cells[particle.getCellX()+1][particle.getCellY()];
        fluidCell& leftCell   = cells[particle.getCellX()-1][particle.getCellY()];

        float dx = particle.getX() - particle.getCellX();
        float dy = particle.getY() - particle.getCellY();

        int leftValid   = leftCell.isWater()  ? 1 : 0;
        int rightValid  = rightCell.isWater() ? 1 : 0;
        int topValid    = topCell.isWater()   ? 1 : 0;
        int bottomValid = bottomCell.isWater()? 1 : 0;

        float sx = 1.0f - dx;
        float sy = 1.0f - dy;

        float horiValid = leftValid*dx + rightValid*sx;
        float vertValid = topValid*dy + bottomValid*sy;

        if(horiValid == 0 && vertValid == 0){continue;}

        float picX = 0;
        float picY = 0;
        float prevGridX = 0;
        float prevGridY = 0;
        float flipX = 0;
        float flipY = 0;
        

        //printf("Weights: %f, %f\n", weightSumX, weightSumY);
        //printf("dx dy: %f, %f\n", dx, dy);
        //printf("valids: %f, %f, %f, %f\n", topValid, bottomValid, leftValid, rightValid);
        //printf("flows: %f, %f, %f, %f\n", currentCell.horizontalFlow, currentCell.verticalFlow, rightCell.horizontalFlow, bottomCell.verticalFlow);
        
        if( horiValid > 0){
            picX = (sx * currentCell.horizontalFlow +
                    dx * rightCell.horizontalFlow )/horiValid;
            prevGridX = (sx * currentCell.prevHorizontalFlow +
                         dx * rightCell.prevHorizontalFlow)/horiValid;
            flipX = particle.vx + (picX - prevGridX);
        }
        if( vertValid > 0){
            picY = (sy * currentCell.verticalFlow +
                    dy * bottomCell.verticalFlow )/vertValid;
            prevGridY = (sy * currentCell.prevVerticalFlow +
                         dy * bottomCell.prevVerticalFlow)/vertValid;
            flipY = particle.vy + (picY - prevGridY);
        }

        // FLIP update: Add the velocity change to the particle


        //printf("Particle %lu at (%u,%u) pic and flip: %f, %f, %f, %f\n", particle.particleId, particle.getCellX(),particle.getCellY(),picX, picY, flipX, flipY);

        particle.vx = ratio * flipX + (1.0f - ratio) * picX;
        particle.vy = ratio * flipY + (1.0f - ratio) * picY;
    }
}

void fluidWindow::printParticles(int iter =  99999){
    for(auto particle: particleArray){
        if(iter--<0){break;}
        printf("Particle id%lu at (%f, %f) cell(%d, %d) with velocity (%f, %f)\n", particle.particleId, particle.getX(), particle.getY(), particle.getCellX(), particle.getCellY(), particle.vx, particle.vy);
    }
}


fluidWindow myWindow;

int main() {
    stdio_init_all(); // Initialize standard IO
    sleep_ms(1000);
    printf("Startup\n");
    sleep_ms(1000);
    myWindow.init();
    printf("Init!\n");
    //myWindow.simulateParticles();
    while (true) {
        //printf("Loop!\n");
        myWindow.simulateParticles();
        myWindow.printParticles();
        //printf("Simulated!\n");
        sleep_ms(10);
        myWindow.toGrid();
        //myWindow.printParticles();
        //printf("Particles to cells!\n");
        sleep_ms(10);
        myWindow.makeIncompressible(50);
        //myWindow.printParticles();
        //printf("Incompressible!\n");   
        sleep_ms(10);
        myWindow.fromGrid(0.9);
        myWindow.printParticles();
        //printf("Cells to particles!\n");    
        //sleep_ms(10);
        myWindow.print();
        sleep_ms(100);
    }
}