// Headless benchmark. Runs every registered algorithm over the committed map
// fixtures and prints a markdown table of expansions, cost, and wall time.

#include "algorithms.h"
#include "grid.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace {

struct BenchMap {
    std::string name;
    Grid grid;
    Point start;
    Point goal;
};

// Fixed list rather than a directory scan so row order is stable across runs
// and platforms, which keeps README diffs clean.
const char* kFixtures[] = {
    "open-s", "open-m", "open-l",
    "maze-s", "maze-m", "maze-l",
    "rooms-s", "rooms-m", "rooms-l",
};

std::optional<BenchMap> loadMap(const std::string& dir, const std::string& name) {
    std::ifstream in(dir + "/" + name + ".txt");
    if (!in) return std::nullopt;

    std::vector<std::string> rows;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!line.empty()) rows.push_back(line);
    }
    if (rows.empty()) return std::nullopt;

    const int w = (int)rows[0].size();
    const int h = (int)rows.size();
    Grid grid(w, h);
    Point start{ -1, -1 }, goal{ -1, -1 };
    for (int y = 0; y < h; ++y) {
        if ((int)rows[y].size() != w) return std::nullopt;
        for (int x = 0; x < w; ++x) {
            switch (rows[y][x]) {
                case '#': grid.set(x, y, Cell::Wall); break;
                case 'S': start = { x, y }; break;
                case 'G': goal = { x, y }; break;
                default: break;
            }
        }
    }
    if (start.x < 0 || goal.x < 0) return std::nullopt;
    return BenchMap{ name, grid, start, goal };
}

long long medianMicros(std::vector<long long>& samples) {
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

}  // namespace

int main(int argc, char** argv) {
    const std::string dir = argc > 1 ? argv[1] : "maps";

    std::printf("| Map | Algorithm | Nodes expanded | Path cost | Time (us) |\n");
    std::printf("|---|---|---:|---:|---:|\n");

    bool loadFailure = false;
    for (const char* fixture : kFixtures) {
        const auto map = loadMap(dir, fixture);
        if (!map) {
            std::fprintf(stderr, "failed to load %s/%s.txt\n", dir.c_str(), fixture);
            loadFailure = true;
            continue;
        }
        for (int a = 0; a < algorithmCount(); ++a) {
            auto algo = makeAlgorithm(a);

            // Median of several runs. A single runToEnd sits in the microsecond
            // range where scheduler noise would dominate one sample.
            std::vector<long long> samples;
            for (int rep = 0; rep < 7; ++rep) {
                algo->init(map->grid, map->start, map->goal);
                const auto t0 = std::chrono::steady_clock::now();
                algo->runToEnd();
                const auto t1 = std::chrono::steady_clock::now();
                samples.push_back(
                    std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count());
            }

            if (algo->status() != SearchStatus::Found) {
                std::fprintf(stderr, "%s found no path on %s\n", algo->name(), fixture);
                loadFailure = true;
                continue;
            }
            const SearchStats st = algo->stats();
            std::printf("| %s | %s | %d | %d | %lld |\n",
                        fixture, algo->name(), st.nodesExpanded, st.pathCost,
                        medianMicros(samples));
        }
    }
    return loadFailure ? 1 : 0;
}
