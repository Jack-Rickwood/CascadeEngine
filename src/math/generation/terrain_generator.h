#include <stdint.h>
#include <externals/FastNoiseLite/FastNoiseLite.h>

namespace cscd {
namespace generation {

class TerrainGenerator {
private:
    FastNoiseLite noise{};

public:
    int mapNoiseToHeight(float value, int terrain_min, int terrain_max);

    void generatePerlin2D(uint8_t* grid, int x_size, int y_size, int z_size);
    void generatePerlin3D(uint8_t* grid, int x_size, int y_size, int z_size);
};

}
}