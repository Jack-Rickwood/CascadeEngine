#include "terrain_generator.h"
#include "physics/particles/particle_types.h"

namespace cscd {
namespace generation {

    void TerrainGenerator::generatePerlin(uint8_t* grid, int x_size, int y_size, int z_size) {
        noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

        for (int x = 0; x < x_size; x++) {
            for (int y = 0; y < y_size; y++) {
                for (int z = 0; z < z_size; z++) {
                    float density = noise.GetNoise<float>(x, y, z);
                    int grid_index = z * x_size * y_size + y * x_size + x;
                    if (density >= 0) {
                        grid[grid_index] = (uint8_t)cscd::physics::ParticleType::SAND;
                    } else {
                        grid[grid_index] = (uint8_t)cscd::physics::ParticleType::EMPTY;
                    }
                }
            }
        }
    }

}
}