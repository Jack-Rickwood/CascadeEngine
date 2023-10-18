#include <stdint.h>
#include <externals/FastNoiseLite/FastNoiseLite.h>

namespace cscd {
namespace generation {

class TerrainGenerator {
private:
    FastNoiseLite noise{};

public:
    void generatePerlin(uint8_t* grid, int x_size, int y_size, int z_size);
};

}
}