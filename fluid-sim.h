#include <array>
#include <tuple>
#include <algorithm>
#include <time.h>
#include "hardware/i2c.h"
#include "pico/multicore.h"
#include "gravity-fields.h"



enum class enumBadgeState {
    zerog,
    normalg,
    displayname1,
    displayname2
} ;

static enumBadgeState currentState{enumBadgeState::normalg};


static constexpr float timeStep = 1.0f/60.0f;
static constexpr int numParticles{350};
static float variance = 10000;

static constexpr std::array<std::array<int, ysize>, xsize> cordsToLedNumber {{ 
    {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1},
    {-1, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443,  -1,  -1,  -1,  -1, -1},
    {-1, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429,  -1,  -1, -1},
    {-1, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414,  -1, -1},
    {-1, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398,  -1, -1},
    {-1, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, -1},
    {-1,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, -1},
    {-1,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, -1},
    {-1,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, -1},
    {-1,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318,  -1, -1},
    {-1,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302,  -1, -1},
    {-1,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285,  -1,  -1, -1},
    {-1,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267,  -1,  -1,  -1,  -1, -1},
    {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1}
}};



static std::array<uint8_t, (12*16)+1> ledBuffer1{};
static std::array<uint8_t, (12*16)+1> ledBuffer2{};

template <typename T>
constexpr T clamp(T value, T min, T max) {
    return (value < min) ? min : (value > max) ? max : value;
}

void outputToDisplay();

enum class cellStateEnum{
    solid,
    air,
    water
};

class fluidCell {
    public:
        cellStateEnum state {cellStateEnum::air};
        float horizontalFlow{0};
        float verticalFlow{0};
        uint32_t numberParticles{0};
        int flowAllowed{1};
        float horizontalWeight{0.0};
        float verticalWeight{0.0};
        float prevHorizontalFlow{0.0};
        float prevVerticalFlow{0.0};
        int x{0};
        int y{0};
        bool isSolid();
        bool isWater();
        bool isAir();
};

class fluidParticle {
    public:
        uint32_t cellNumber{0};
        float vx{0};
        float vy{0};
        float diameter{1};  
        uint32_t particleId{0};   
        void setCoordinates(float newX, float newY);
        void setCell(uint8_t cellx, uint8_t celly);
        float getX();
        float getY();
        uint8_t getCellX();
        uint8_t getCellY();
    private:   
        float x{0};
        float y{0};
        int cellX{0};
        int cellY{0};
};

void printParticle(fluidParticle& particle, const char* message){
    printf("Particle id %u at (%f, %f) with velocity (%f, %f), %s.\n", particle.particleId, particle.getX(), particle.getY(), particle.vx, particle.vy, message);
}

class fluidWindow {
    public:
        fluidWindow(){};
        int loopNumber = 0;
        static constexpr float particleDensity{numParticles/((xsize-2.0f)*(ysize-2.0f))};
        std::array<std::array<fluidCell, ysize>, xsize> cells;
        std::array<fluidParticle, numParticles> particleArray;
        std::array<uint32_t, numParticles> particlePointers;
        std::array<uint32_t, ysize*xsize+1> cellParticleCount;
        uint32_t getCellNumberFromParticle(float x, float y);
        uint32_t getCellNumberFromCords(uint8_t x, uint8_t y);
        std::tuple<uint16_t, uint16_t> getParticleStats(uint32_t cellNumber);
        void print();
        void printParticles(int iter);
        void init();
        void updateDataStructures();
        void simulateParticles();
        fluidCell& getCell(fluidParticle& particle);
        fluidCell& getCell(uint8_t x, uint8_t y);
        fluidCell& right(fluidCell& cell);
        fluidCell& left(fluidCell& cell);
        fluidCell& up(fluidCell& cell);
        fluidCell& down(fluidCell& cell);
        uint32_t getParticlesInCell(fluidParticle& particle);
        void checkCollision(fluidParticle& particle1, fluidParticle& particle2);
        void particlesToCells();
        void toGrid();
        void makeIncompressible(uint8_t iterations);
        void cellsToParticles(float ratio);
        void fromGrid(float ratio);
        void handleSolidCells();
        void handleParticleCollisions();
        void integrateParticles();
        void stepSim();
        std::array<uint8_t,(xsize-2)*(ysize-2)-12> ledCommand{};
};

std::array<std::array<uint8_t, ysize>, xsize> brigtness_array;


// Use the updated constants from fluid-sim.h
constexpr int GRID_X = xsize;  // 13 (X is first index)
constexpr int GRID_Y = ysize;  // 29 (Y is second index)
constexpr int UPSCALE = 3;     // 3x resolution increase
constexpr int HIGH_X = GRID_X * UPSCALE;  // 39
constexpr int HIGH_Y = GRID_Y * UPSCALE;  // 87

// Computes the gravitational force at a high-resolution grid point
constexpr std::pair<float, float> computeForceAt(int x, int y, const std::array<std::array<uint8_t, GRID_Y>, GRID_X>& gravityField) {
    float forceX = 0.0f, forceY = 0.0f;
    constexpr float strength = 10.0f;  // Attraction strength (tweak as needed)
    
    for (int gx = 0; gx < GRID_X; ++gx) {
        for (int gy = 0; gy < GRID_Y; ++gy) {
            if (gravityField[gx][gy]) {  // Attractor point exists
                float dx = (gx * UPSCALE + UPSCALE / 2) - x;
                float dy = (gy * UPSCALE + UPSCALE / 2) - y;
                float dist2 = dx * dx + dy * dy + 1.0f;  // Prevent division by zero
                float dist4 = dx * dx * dx * dx + dy * dy * dy * dy + 1.0f;
                float forceMag = strength * gravityField[gx][gy] / dist4;  // 1 / r² falloff
                forceX += dx * forceMag;
                forceY += dy * forceMag;
            }
        }
    }
    
    return {forceX, forceY};
}

// Generates the high-resolution gravity field at compile-time
constexpr auto generateGravityField(const std::array<std::array<uint8_t, GRID_Y>, GRID_X>& gravityField) {
    std::array<std::array<std::pair<float, float>, HIGH_Y>, HIGH_X> output = {};
    
    for (int x = 0; x < HIGH_X; ++x) {
        for (int y = 0; y < HIGH_Y; ++y) {
            output[x][y] = computeForceAt(x, y, gravityField);
        }
    }
    
    return output;
}

#define I2C_PORT i2c1
#define SDA_PIN  2
#define SCL_PIN  3
#define CHIP_1 0b1010011  // Default I2C address
#define CHIP_2 0b1010000  // Default I2C address

void recover_i2c_bus() {
    gpio_set_function(SCL_PIN, GPIO_FUNC_SIO); // Switch SCL to GPIO mode
    gpio_set_function(SDA_PIN, GPIO_FUNC_SIO); // Switch SDA to GPIO mode

    gpio_set_dir(SCL_PIN, GPIO_OUT);
    gpio_set_dir(SDA_PIN, GPIO_OUT);

    // Generate 9 clock pulses on SCL
    for (int i = 0; i < 20; i++) {
        gpio_put(SCL_PIN, 1);
        sleep_us(10);  // Short delay
        gpio_put(SCL_PIN, 0);
        sleep_us(10);
    }

    // Send a STOP condition to reset slaves
    gpio_put(SDA_PIN, 1);
    gpio_put(SCL_PIN, 1);
    sleep_us(5);

    // Restore I2C functionality
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
}

void reset_i2c() {
    i2c_deinit(I2C_PORT);
    i2c_init(I2C_PORT, 400 * 1000);  // Reinitialize at 400 kHz
}


void i2c_write_chip1(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    i2c_write_timeout_us(I2C_PORT, CHIP_1, data, 2, false, 1000);
}

void i2c_write_chip2(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    i2c_write_timeout_us(I2C_PORT, CHIP_2, data, 2, false, 1000);
}

#define LIS3DH_ADDR 0x19  // I2C address when SDO is high
#define OUT_X_L 0x28      // First acceleration register

void i2c_read_accel(int16_t &x, int16_t &y, int16_t &z) {
    uint8_t reg = OUT_X_L | 0x80;  // Set MSB to enable auto-increment
    uint8_t data[6];  // Buffer for X, Y, Z (low & high bytes)

    // Write the register address
    if (i2c_write_timeout_us(I2C_PORT, LIS3DH_ADDR, &reg, 1, false, 1000) == PICO_ERROR_TIMEOUT) {
        printf("I2C write failed\n");
        recover_i2c_bus();
        reset_i2c();
        return;
    }

    // Read 6 bytes (X_L, X_H, Y_L, Y_H, Z_L, Z_H)
    if (i2c_read_timeout_us(I2C_PORT, LIS3DH_ADDR, data, 6, false, 1000) == PICO_ERROR_TIMEOUT) {
        printf("I2C read failed\n");
        recover_i2c_bus();
        reset_i2c();
        return;
    }

    // Convert to 16-bit signed values
    y = -(int16_t)(data[1] << 8 | data[0]);
    x = -(int16_t)(data[3] << 8 | data[2]);
    z = (int16_t)(data[5] << 8 | data[4]);
}



void is31fl3733_init() {
    // Select function page
    i2c_write_chip1(0xFE, 0xC5);
    i2c_write_chip1(0xFD, 0x03);
    i2c_write_chip2(0xFE, 0xC5);
    i2c_write_chip2(0xFD, 0x03);


    // Set Global Current Control Register
    i2c_write_chip1(0x01, 0x80);
    i2c_write_chip2(0x01, 0x80);

    // Set Enable chip
    i2c_write_chip1(0x00, 0x01);
    i2c_write_chip2(0x00, 0x01);

    // Select LED control Register Page
    i2c_write_chip1(0xFE, 0xC5);
    i2c_write_chip1(0xFD, 0x00);
    i2c_write_chip2(0xFE, 0xC5);
    i2c_write_chip2(0xFD, 0x00);

    // Enable all installed LEDs on Chip 1
    for(int i = 0; i < 12; i++){
        i2c_write_chip1(2*i, 0xFF);
        i2c_write_chip1(2*i + 1, 0xFF);
    }

    // Enable all full rows on Chip 2
    for(int i = 0; i < 12; i++){
        i2c_write_chip2(2*i, 0xFF);
    }
    i2c_write_chip2(1, 0x0F);
    i2c_write_chip2(3, 0x3F);
    i2c_write_chip2(5, 0x7F);
    i2c_write_chip2(7, 0x7F);
    i2c_write_chip2(9, 0xFF);
    i2c_write_chip2(11, 0xFF);
    i2c_write_chip2(13, 0xFF);
    i2c_write_chip2(15, 0xFF);
    i2c_write_chip2(17, 0x7F);
    i2c_write_chip2(19, 0x7F);
    i2c_write_chip2(21, 0x3F);
    i2c_write_chip2(23, 0x0F);

    // Select page 1
    i2c_write_chip1(0xFE, 0xC5);
    i2c_write_chip1(0xFD, 0x01);  
    i2c_write_chip2(0xFE, 0xC5);
    i2c_write_chip2(0xFD, 0x01);  

    // Enable accelerometer
    uint8_t data[2] = {0x20, 0x57};  // Normal power mode, all axes enabled
    i2c_write_timeout_us(I2C_PORT, LIS3DH_ADDR, data, 2, false, 1000);
   
    data[0] = 0x23;
    data[1] = 0x20;  // 8g full scale
    i2c_write_timeout_us(I2C_PORT, LIS3DH_ADDR, data, 2, false, 1000);
}

void set_all_brightness(){
    if(i2c_write_timeout_us(I2C_PORT, CHIP_1, ledBuffer1.data(), sizeof(ledBuffer1), false, 10000) == PICO_ERROR_TIMEOUT){
        printf("LED DRIVER WRITE TIMEOUT!!!\n");
        sleep_ms(1);
        recover_i2c_bus();
        reset_i2c();
    };
    if(i2c_write_timeout_us(I2C_PORT, CHIP_2, ledBuffer2.data(), sizeof(ledBuffer2), false, 10000) == PICO_ERROR_TIMEOUT){
        printf("LED DRIVER WRITE TIMEOUT!!!\n");
        sleep_ms(1);
        recover_i2c_bus();
        reset_i2c();
    };
}
