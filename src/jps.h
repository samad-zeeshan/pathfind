// Jump Point Search: A* that skips the grid's symmetric paths by jumping down
// straight and diagonal runs, queueing only cells where an optimal path can
// turn. Assumes the uniform-cost octile grid with 8-direction movement.

#pragma once

#include "search_core.h"

class JpsSearch : public HeapSearch {
public:
    JpsSearch() : HeapSearch(kOctileCosts) {}
    const char* name() const override { return "JPS"; }

protected:
    int priority(int g, int h) const override { return g + h; }
    void expandNode(int idx, int g) override;

    // Parents are jump points several cells apart, the path between them is
    // filled back in so the result meets the same invariants as everyone else.
    void finishFromParents() override;

private:
    bool freeAt(int x, int y) const;
    bool canMove(int x, int y, int dx, int dy) const;
    int jumpStraight(int x, int y, int dx, int dy) const;
    int jumpDiagonal(int x, int y, int dx, int dy) const;
    void jumpFrom(int idx, int g, int x, int y, int dx, int dy);
};
