// Dijkstra: A* with the heuristic switched off. Optimal, but explores in every
// direction equally.

#pragma once

#include "search_core.h"

class DijkstraSearch : public HeapSearch {
public:
    explicit DijkstraSearch(CostModel cost = kOctileCosts) : HeapSearch(cost) {}
    const char* name() const override { return "Dijkstra"; }

protected:
    int priority(int g, int /*h*/) const override { return g; }
};
