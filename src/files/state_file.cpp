#include <fstream>
#include <iterator>
#include <stdexcept>
#include "state_file.h"

cscd::file::State::State(std::string path) {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Failed to open file!");
    }

    char magic[5];
    file.read(magic, 4);
    magic[4] = '\0';

    if (strcmp(magic, STATE_MAGIC)) {
        throw std::runtime_error("Invalid file magic!");
    }

    file.read((char*)&x_size, 2);
    file.read((char*)&y_size, 2);
    file.read((char*)&z_size, 2);
    int size = x_size * y_size * z_size;

    if (size <= 0) {
        throw std::runtime_error("File contains no data!");
    }

    data.resize(size);
    file.read(reinterpret_cast<char*>(data.data()), size);

    file.close();
}

void cscd::file::State::writeToFile(std::string path) {
    int size = x_size * y_size * z_size;
    std::ofstream file(path, std::ios::binary);

    if (size <= 0) {
        throw std::runtime_error("State struct contains no data!");
    } else if (size != data.size()) {
        throw std::runtime_error("Provided data doesn't match size values!");
    } else if (!file) {
        throw std::runtime_error("Failed to open file!");
    }

    file.write(STATE_MAGIC, 4);
    file.write((char*)&x_size, 2);
    file.write((char*)&y_size, 2);
    file.write((char*)&z_size, 2);
    file.write(reinterpret_cast<char*>(data.data()), size);

    file.close();
}

void cscd::file::State::setSize(uint16_t x_size_, uint16_t y_size_, uint16_t z_size_) {
    x_size = x_size_;
    y_size = y_size_;
    z_size = z_size_;
    data.resize(x_size * y_size * z_size);
}

int cscd::file::State::getSize() {
    int size = x_size * y_size * z_size;
    if (size != data.size()) {
        throw std::runtime_error("Data doesn't match size values!");
    }
    return size;
}

glm::ivec3 cscd::file::State::getDimensions() {
    return glm::ivec3{x_size, y_size, z_size};
}

uint8_t cscd::file::State::read(int x, int y, int z) {
    int index = z * y_size * x_size + y * x_size + x;
    return data[index];
}

void cscd::file::State::write(int x, int y, int z, uint8_t byte) {
    int index = z * y_size * x_size + y * x_size + x;
    data[index] = byte;
}