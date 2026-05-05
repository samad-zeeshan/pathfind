// Greedy best-first: ordered by the heuristic alone. Fast because it commits to
// whatever looks closest to the goal, and not optimal for the same reason.

#pragma once

#include "search_core.h"

class GreedySearch : public HeapSearch {
public:
    explicit GreedySearch(CostModel cost = kOctileCosts) : HeapSearch(cost) {}
    const char* name() const override { return "Greedy"; }

protected:
    int priority(int /*g*/, int h) const override { return h; }

    // Discover each cell once. Greedy ignores g when ordering, so a better g
    // would never change its choices, requeueing would only duplicate entries.
    bool tryRelax(int nIdx, int newG) override {
        if (g_[nIdx] != kSearchInf) return false;
        g_[nIdx] = newG;
        return true;
    }
};
