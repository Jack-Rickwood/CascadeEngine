#include "terrain_generator.h"
#include "physics/particles/particle_types.h"

namespace cscd {
namespace generation {

    int TerrainGenerator::mapNoiseToHeight(float value, int terrain_min, int terrain_max) {
        return terrain_min + value * (terrain_max - terrain_min);
    }

    void TerrainGenerator::generatePerlin2D(uint8_t* grid, int x_size, int y_size, int z_size) {
        noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFractalOctaves(8);

        float scale = 0.25f;
        for (int x = 0; x < x_size; x++) {
            for (int z = 0; z < z_size; z++) {
                float terrain_ratio = (1.0f / 2.0f);
                int terrain_min = (terrain_ratio / 2.0f) * y_size;
                int terrain_max = (1.0f - (terrain_ratio / 2.0f)) * y_size;
                int height = mapNoiseToHeight(noise.GetNoise<float>(x * scale, z * scale), terrain_min, terrain_max);
                for (int y = 0; y < y_size; y++) {
                    int grid_index = z * x_size * y_size + y * x_size + x;
                    if (y < height) {
                        grid[grid_index] = (uint8_t)cscd::physics::ParticleType::SAND;
                    } else {
                        grid[grid_index] = (uint8_t)cscd::physics::ParticleType::AIR;
                    }
                }
            }
        }

        
        for (int x = 0; x < x_size; x++) {
            for (int z = 0; z < z_size; z++) {
                float terrain_ratio = (1.0f / 2.0f);
                int terrain_min = (terrain_ratio / 2.0f) * y_size;
                int terrain_max = (1.0f - (terrain_ratio / 2.0f)) * y_size;
                int height = mapNoiseToHeight(noise.GetNoise<float>(x, z), terrain_min, terrain_max);
                for (int y = y_size / 2; y < y_size; y++) {
                    int grid_index = z * x_size * y_size + y * x_size + x;
                    if (y < (2 + y_size / 2)) {
                        grid[grid_index] = (uint8_t)cscd::physics::ParticleType::WATER;
                    } else {
                        grid[grid_index] = (uint8_t)cscd::physics::ParticleType::AIR;
                    }
                }
            }
        }
        
       /*
        for (int x = (2 * x_size/5); x < (3 * x_size/5); x++) {
            for (int z = (2 * z_size/5); z < (3 * z_size/5); z++) {
                float terrain_ratio = (1.0f / 2.0f);
                int terrain_min = (terrain_ratio / 2.0f) * y_size;
                int terrain_max = (1.0f - (terrain_ratio / 2.0f)) * y_size;
                int height = mapNoiseToHeight(noise.GetNoise<float>(x, z), terrain_min, terrain_max);
                for (int y = y_size/2; y < y_size; y++) {
                    int grid_index = z * x_size * y_size + y * x_size + x;
                    if (y < y_size) {
                        grid[grid_index] = (uint8_t)cscd::physics::ParticleType::SAND;
                    } else {
                        grid[grid_index] = (uint8_t)cscd::physics::ParticleType::AIR;
                    }
                }
            }
        }
        */

        for (int x = x_size - 10 - 1; x < x_size; x++) {
            for (int z = z_size - 10 - 1; z < z_size; z++) {
                for (int y = y_size - 10 - 1; y < y_size; y++) {
                    int grid_index = z * x_size * y_size + y * x_size + x;
                    grid[grid_index] = (uint8_t)cscd::physics::ParticleType::SUN;
                }
            }
        }
    }

    void TerrainGenerator::generatePerlin3D(uint8_t* grid, int x_size, int y_size, int z_size) {
        noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFractalOctaves(6);

        for (int x = 0; x < x_size; x++) {
            for (int y = 0; y < y_size; y++) {
                for (int z = 0; z < z_size; z++) {
                    float density = noise.GetNoise<float>(x, y, z);
                    int grid_index = z * x_size * y_size + y * x_size + x;
                    if (density >= 0) {
                        grid[grid_index] = (uint8_t)cscd::physics::ParticleType::SAND;
                    } else {
                        grid[grid_index] = (uint8_t)cscd::physics::ParticleType::AIR;
                    }
                }
            }
        }
    }

}
}