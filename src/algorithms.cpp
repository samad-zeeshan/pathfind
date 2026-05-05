// The registry table. Order matters, it is the story the demo tells: BFS ignores
// weights, Dijkstra is optimal but broad, Greedy is fast but wrong, A* is both.

#include "algorithms.h"

#include <cstring>

#include "bfs.h"
#include "dijkstra.h"
#include "greedy.h"
#include "astar.h"
#include "bidirectional.h"
#include "jps.h"

namespace {

using Factory = std::unique_ptr<Pathfinder> (*)();

struct Entry {
    const char* name;
    Factory make;
};

template <typename T>
std::unique_ptr<Pathfinder> create() { return std::make_unique<T>(); }

constexpr Entry kAlgorithms[] = {
    { "BFS",      create<BfsSearch> },
    { "Dijkstra", create<DijkstraSearch> },
    { "Greedy",   create<GreedySearch> },
    { "A*",       create<AStarSearch> },
    { "Bi-A*",    create<BidirAStarSearch> },
    { "JPS",      create<JpsSearch> },
};

}  // namespace

int algorithmCount() {
    return (int)(sizeof(kAlgorithms) / sizeof(kAlgorithms[0]));
}

const char* algorithmName(int index) {
    return kAlgorithms[index].name;
}

int algorithmIndex(const char* name) {
    for (int i = 0; i < algorithmCount(); ++i) {
        if (std::strcmp(kAlgorithms[i].name, name) == 0) return i;
    }
    return -1;
}

std::unique_ptr<Pathfinder> makeAlgorithm(int index) {
    return kAlgorithms[index].make();
}
