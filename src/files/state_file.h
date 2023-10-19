#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vector>
#include <string>
#include <cstring>
#include <stdint.h>
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "math/generation/terrain_generator.h"

#define STATE_MAGIC "CCST"

namespace cscd {
namespace file {

struct State {
    uint16_t x_size = 0;
    uint16_t y_size = 0;
    uint16_t z_size = 0;
    std::vector<uint8_t> data;

    cscd::generation::TerrainGenerator generator{};

    State() = delete;
    State(uint16_t x_size_, uint16_t y_size_, uint16_t z_size_, uint8_t* data_) :
        x_size{x_size_},
        y_size{y_size_},
        z_size{z_size_}
    {
        int size = x_size_ * y_size_ * z_size_;
        data.resize(size);
        std::memcpy(data.data(), data_, size);
    }
    State(uint16_t x_size_, uint16_t y_size_, uint16_t z_size_) :
        x_size{x_size_},
        y_size{y_size_},
        z_size{z_size_}
    {
        int size = x_size_ * y_size_ * z_size_;
        data.resize(size);
    }
    State(std::string path);

    void setSize(uint16_t x_size_, uint16_t y_size_, uint16_t z_size_);
    int getSize();
    glm::ivec3 getDimensions();
    uint8_t read(int x, int y, int z);
    void write(int x, int y, int z, uint8_t byte);
    void writeToFile(std::string path);

    void fillPerlin() { generator.generatePerlin2D(data.data(), x_size, y_size, z_size); }
};

}
}