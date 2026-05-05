// A*: best-first on g plus an admissible heuristic. Optimal like Dijkstra but
// focused toward the goal.

#pragma once

#include "search_core.h"

class AStarSearch : public HeapSearch {
public:
    explicit AStarSearch(CostModel cost = kOctileCosts) : HeapSearch(cost) {}
    const char* name() const override { return "A*"; }

protected:
    int priority(int g, int h) const override { return g + h; }
};

struct AStarResult {
    std::vector<Point> path;
    int nodesExpanded = 0;
    bool found = false;
};

inline AStarResult astar(const Grid& grid, int sx, int sy, int gx, int gy) {
    AStarSearch s;
    s.init(grid, { sx, sy }, { gx, gy });
    s.runToEnd();
    AStarResult r;
    r.found         = s.status() == SearchStatus::Found;
    r.nodesExpanded = s.stats().nodesExpanded;
    if (r.found) r.path = s.path();
    return r;
}
