#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include "graphics/application/application.h"
#include "files/state_file.h"

void writeExampleState() {
    cscd::file::State world_state{3, 3, 3};

    for (int x = 0; x < world_state.x_size; x++) {
        for (int y = 0; y < world_state.y_size; y++) {
            for (int z = 0; z < world_state.z_size; z++) {
                if (x == 1 && y == 1 && z == 1) {
                    world_state.write(x, y, z, 0x1);
                } else {
                    world_state.write(x, y, z, 0x0);
                }
            }
        }
    }

    world_state.writeToFile("state.ccst");
}

void writeExampleStatePerlin() {
    cscd::file::State world_state{64, 64, 64};

    world_state.fillPerlin();
    world_state.writeToFile("state.ccst");
}

int main() {
    writeExampleStatePerlin();

    cscd::Application app{"state.ccst"};

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}