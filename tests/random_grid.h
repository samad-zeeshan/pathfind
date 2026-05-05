// Deterministic random maps for the property tests.

#pragma once

#include "grid.h"
#include "pathfinder.h"

#include <cstdint>
#include <random>

struct TestMap {
    Grid grid;
    Point start;
    Point goal;
};

// mt19937 with raw modulo instead of <random> distributions, which are not
// specified identically across standard libraries. Same seed, same map, on
// every platform.
inline TestMap randomMap(uint32_t seed, int w, int h, int wallPercent) {
    std::mt19937 rng(seed);
    Grid grid(w, h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if ((int)(rng() % 100) < wallPercent) grid.set(x, y, Cell::Wall);
        }
    }
    Point start{ (int)(rng() % w), (int)(rng() % h) };
    Point goal = start;
    while (goal == start) {
        goal = { (int)(rng() % w), (int)(rng() % h) };
    }
    grid.set(start.x, start.y, Cell::Floor);
    grid.set(goal.x, goal.y, Cell::Floor);
    return { grid, start, goal };
}
