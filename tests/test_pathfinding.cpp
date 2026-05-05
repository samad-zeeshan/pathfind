// Property tests for every registered algorithm, plus the cross-algorithm
// oracle that grounds correctness in cost agreement between optimal searches.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "algorithms.h"
#include "astar.h"
#include "bfs.h"
#include "bidirectional.h"
#include "dijkstra.h"
#include "greedy.h"
#include "grid.h"
#include "jps.h"
#include "random_grid.h"

#include <cstdlib>
#include <memory>
#include <vector>

namespace {

bool adjacent(Point a, Point b) {
    const int dx = std::abs(a.x - b.x);
    const int dy = std::abs(a.y - b.y);
    return (dx != 0 || dy != 0) && dx <= 1 && dy <= 1;
}

// The per-path invariants every algorithm must satisfy: endpoints match, every
// cell is walkable, consecutive cells are adjacent, diagonals never cut corners.
void requireValidPath(const Pathfinder& algo, const Grid& grid, Point start, Point goal) {
    const std::vector<Point>& path = algo.path();
    REQUIRE(!path.empty());
    CHECK(path.front() == start);
    CHECK(path.back() == goal);
    for (size_t i = 0; i < path.size(); ++i) {
        CHECK(grid.at(path[i].x, path[i].y) == Cell::Floor);
        if (i == 0) continue;
        const Point a = path[i - 1];
        const Point b = path[i];
        CHECK(adjacent(a, b));
        if (a.x != b.x && a.y != b.y) {
            CHECK(grid.at(b.x, a.y) == Cell::Floor);
            CHECK(grid.at(a.x, b.y) == Cell::Floor);
        }
    }
    CHECK((int)path.size() == algo.stats().pathLength);
}

// Map sizes and wall densities are cycled so the suite covers open fields,
// typical mazes, and mostly-blocked grids.
TestMap mapForCase(uint32_t seed) {
    const int sizes[][2] = { { 16, 12 }, { 24, 24 }, { 40, 30 }, { 48, 36 } };
    const int density[] = { 10, 20, 30, 45 };
    return randomMap(seed,
                     sizes[seed % 4][0], sizes[seed % 4][1],
                     density[(seed / 4) % 4]);
}

}  // namespace

TEST_CASE("every algorithm: path invariants and determinism on random maps") {
    for (int a = 0; a < algorithmCount(); ++a) {
        CAPTURE(algorithmName(a));
        for (uint32_t seed = 0; seed < 60; ++seed) {
            CAPTURE(seed);
            const TestMap m = mapForCase(seed);

            auto first = makeAlgorithm(a);
            first->init(m.grid, m.start, m.goal);
            const SearchStatus result = first->runToEnd();
            REQUIRE(result != SearchStatus::Running);
            if (result == SearchStatus::Found) {
                requireValidPath(*first, m.grid, m.start, m.goal);
            }

            // Same inputs must reproduce the identical search, not just an
            // equally good one.
            auto second = makeAlgorithm(a);
            second->init(m.grid, m.start, m.goal);
            second->runToEnd();
            REQUIRE(second->status() == result);
            CHECK(second->stats().nodesExpanded == first->stats().nodesExpanded);
            REQUIRE(second->path().size() == first->path().size());
            for (size_t i = 0; i < first->path().size(); ++i) {
                CHECK(second->path()[i] == first->path()[i]);
            }
        }
    }
}

TEST_CASE("oracle: optimal algorithms agree on cost, greedy never beats them") {
    int foundCount = 0;
    for (uint32_t seed = 100; seed < 260; ++seed) {
        CAPTURE(seed);
        const TestMap m = mapForCase(seed);

        AStarSearch astar;
        astar.init(m.grid, m.start, m.goal);
        astar.runToEnd();

        // Every optimal algorithm must reproduce A*'s cost exactly. This is
        // the strongest check in the suite, a wrong termination condition in
        // Bi-A* or a bad pruning rule in JPS shows up here immediately.
        DijkstraSearch dijkstra;
        BidirAStarSearch bidir;
        JpsSearch jps;
        Pathfinder* optimal[] = { &dijkstra, &bidir, &jps };
        for (Pathfinder* algo : optimal) {
            CAPTURE(algo->name());
            algo->init(m.grid, m.start, m.goal);
            algo->runToEnd();
            CHECK(algo->status() == astar.status());
            if (astar.status() == SearchStatus::Found) {
                CHECK(algo->stats().pathCost == astar.stats().pathCost);
            }
        }

        GreedySearch greedy;
        greedy.init(m.grid, m.start, m.goal);
        greedy.runToEnd();
        CHECK(greedy.status() == astar.status());

        if (astar.status() != SearchStatus::Found) continue;
        ++foundCount;
        CHECK(greedy.stats().pathCost >= astar.stats().pathCost);
    }
    // Guard against the generator silently producing only unreachable maps.
    CHECK(foundCount >= 60);
}

TEST_CASE("oracle: BFS matches Dijkstra when every move costs one") {
    for (uint32_t seed = 300; seed < 420; ++seed) {
        CAPTURE(seed);
        const TestMap m = mapForCase(seed);

        BfsSearch bfs;
        DijkstraSearch unitDijkstra(kUnitCosts);
        bfs.init(m.grid, m.start, m.goal);
        unitDijkstra.init(m.grid, m.start, m.goal);
        bfs.runToEnd();
        unitDijkstra.runToEnd();

        REQUIRE(bfs.status() == unitDijkstra.status());
        if (bfs.status() != SearchStatus::Found) continue;
        CHECK(bfs.stats().pathCost == unitDijkstra.stats().pathCost);
        CHECK(bfs.stats().pathLength == unitDijkstra.stats().pathLength);
    }
}

TEST_CASE("reachability agreement across the whole registry") {
    for (uint32_t seed = 500; seed < 560; ++seed) {
        CAPTURE(seed);
        const TestMap m = mapForCase(seed);

        auto reference = makeAlgorithm(0);
        reference->init(m.grid, m.start, m.goal);
        const SearchStatus expected = reference->runToEnd();
        for (int a = 1; a < algorithmCount(); ++a) {
            CAPTURE(algorithmName(a));
            auto algo = makeAlgorithm(a);
            algo->init(m.grid, m.start, m.goal);
            CHECK(algo->runToEnd() == expected);
        }
    }
}

TEST_CASE("open grid: cost equals the octile distance formula") {
    Grid grid(30, 20);
    const Point start{ 2, 3 };
    const Point goal{ 27, 15 };
    const int expected = octileDistance(goal.x - start.x, goal.y - start.y, kOctileCosts);

    AStarSearch astar;
    astar.init(grid, start, goal);
    REQUIRE(astar.runToEnd() == SearchStatus::Found);
    CHECK(astar.stats().pathCost == expected);

    DijkstraSearch dijkstra;
    dijkstra.init(grid, start, goal);
    REQUIRE(dijkstra.runToEnd() == SearchStatus::Found);
    CHECK(dijkstra.stats().pathCost == expected);
}

TEST_CASE("degenerate cases") {
    SUBCASE("start equals goal") {
        Grid grid(8, 8);
        for (int a = 0; a < algorithmCount(); ++a) {
            CAPTURE(algorithmName(a));
            auto algo = makeAlgorithm(a);
            algo->init(grid, { 3, 3 }, { 3, 3 });
            REQUIRE(algo->runToEnd() == SearchStatus::Found);
            REQUIRE(algo->path().size() == 1);
            const Point origin{ 3, 3 };
            CHECK(algo->path().front() == origin);
            CHECK(algo->stats().pathCost == 0);
        }
    }

    SUBCASE("endpoint on a wall is NoPath before any step") {
        Grid grid(8, 8);
        grid.set(2, 2, Cell::Wall);
        for (int a = 0; a < algorithmCount(); ++a) {
            CAPTURE(algorithmName(a));
            auto algo = makeAlgorithm(a);
            algo->init(grid, { 2, 2 }, { 5, 5 });
            CHECK(algo->status() == SearchStatus::NoPath);
        }
    }

    SUBCASE("diagonal gap between two walls is not a path") {
        // S #
        // # G   the only route is the corner cut, which the rules forbid.
        Grid grid(2, 2);
        grid.set(1, 0, Cell::Wall);
        grid.set(0, 1, Cell::Wall);
        for (int a = 0; a < algorithmCount(); ++a) {
            CAPTURE(algorithmName(a));
            auto algo = makeAlgorithm(a);
            algo->init(grid, { 0, 0 }, { 1, 1 });
            CHECK(algo->runToEnd() == SearchStatus::NoPath);
        }
    }

    SUBCASE("walled-off goal is NoPath for everyone") {
        Grid grid(12, 12);
        for (int d = 0; d < 8; ++d) {
            grid.set(8 + kDX[d], 8 + kDY[d], Cell::Wall);
        }
        for (int a = 0; a < algorithmCount(); ++a) {
            CAPTURE(algorithmName(a));
            auto algo = makeAlgorithm(a);
            algo->init(grid, { 1, 1 }, { 8, 8 });
            CHECK(algo->runToEnd() == SearchStatus::NoPath);
        }
    }
}
