#include "rng.h"

typedef std::mt19937 rng_type;

int Rand::range(int min, int max) {
    static std::minstd_rand gen(std::random_device{}());
    std::uniform_int_distribution<std::minstd_rand::result_type> dist(min,max);
    return dist(gen);
}