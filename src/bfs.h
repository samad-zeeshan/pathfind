// Breadth-first search: a plain FIFO frontier that ignores move costs. Only
// optimal when distance means hop count, which a diagonal grid's does not.

#pragma once

#include "search_core.h"

#include <queue>

class BfsSearch : public GridSearchBase {
public:
    BfsSearch() : GridSearchBase(kUnitCosts) {}
    SearchStatus step() override;
    const char* name() const override { return "BFS"; }

private:
    std::queue<int> queue_;

    void resetDerived() override { queue_ = {}; }
    void seed(int startIdx) override;
};
