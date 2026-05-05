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
