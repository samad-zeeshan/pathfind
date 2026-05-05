// JPS implementation, adapted for the no-corner-cutting movement rule.
//
// The adaptation changes the pruning geometry from canonical JPS. Because a
// diagonal move already requires both flanking cells free, diagonal steps have
// no forced neighbors at all. For straight steps the classic symmetric shortcut
// (diagonally past the wall) is illegal for every path, so the forced-neighbor
// trigger moves to the cell diagonally *behind*: going right, the cell up-left
// being blocked with the cell above free means paths that used to slip through
// up-left now have to turn here, making this cell a jump point.

#include "jps.h"

#include "grid.h"

#include <algorithm>
#include <vector>

bool JpsSearch::freeAt(int x, int y) const {
    return grid_->inBounds(x, y) && grid_->at(x, y) != Cell::Wall;
}

bool JpsSearch::canMove(int x, int y, int dx, int dy) const {
    if (!freeAt(x + dx, y + dy)) return false;
    if (dx != 0 && dy != 0) {
        if (!freeAt(x + dx, y)) return false;
        if (!freeAt(x, y + dy)) return false;
    }
    return true;
}

int JpsSearch::jumpStraight(int x, int y, int dx, int dy) const {
    while (canMove(x, y, dx, dy)) {
        x += dx;
        y += dy;
        const int idx = index(x, y);
        if (idx == goalIdx_) return idx;
        if (dx != 0) {
            if ((!freeAt(x - dx, y + 1) && freeAt(x, y + 1)) ||
                (!freeAt(x - dx, y - 1) && freeAt(x, y - 1))) return idx;
        } else {
            if ((!freeAt(x + 1, y - dy) && freeAt(x + 1, y)) ||
                (!freeAt(x - 1, y - dy) && freeAt(x - 1, y))) return idx;
        }
    }
    return -1;
}

int JpsSearch::jumpDiagonal(int x, int y, int dx, int dy) const {
    while (canMove(x, y, dx, dy)) {
        x += dx;
        y += dy;
        const int idx = index(x, y);
        if (idx == goalIdx_) return idx;
        // A diagonal cell is a jump point when either straight probe finds one,
        // otherwise the probes would be lost when the diagonal run moves on.
        if (jumpStraight(x, y, dx, 0) != -1) return idx;
        if (jumpStraight(x, y, 0, dy) != -1) return idx;
    }
    return -1;
}

void JpsSearch::jumpFrom(int idx, int g, int x, int y, int dx, int dy) {
    const int jp = (dx != 0 && dy != 0) ? jumpDiagonal(x, y, dx, dy)
                                        : jumpStraight(x, y, dx, dy);
    if (jp == -1) return;
    // Jump segments are pure straight or pure diagonal runs, so the octile
    // formula gives their exact cost.
    const int cost = octileDistance(jp % W_ - x, jp / W_ - y, cost_);
    enqueue(idx, jp, g + cost);
}

void JpsSearch::expandNode(int idx, int g) {
    const int x = idx % W_;
    const int y = idx / W_;
    const int p = parent_[idx];

    if (p < 0) {
        // The start has no arrival direction, every direction is worth a jump.
        for (int d = 0; d < 8; ++d) {
            jumpFrom(idx, g, x, y, kDX[d], kDY[d]);
        }
        return;
    }

    const int px = p % W_;
    const int py = p / W_;
    const int dx = (x > px) - (x < px);
    const int dy = (y > py) - (y < py);

    if (dx != 0 && dy != 0) {
        jumpFrom(idx, g, x, y, dx, 0);
        jumpFrom(idx, g, x, y, 0, dy);
        jumpFrom(idx, g, x, y, dx, dy);
    } else if (dx != 0) {
        jumpFrom(idx, g, x, y, dx, 0);
        if (!freeAt(x - dx, y + 1) && freeAt(x, y + 1)) {
            jumpFrom(idx, g, x, y, 0, 1);
            jumpFrom(idx, g, x, y, dx, 1);
        }
        if (!freeAt(x - dx, y - 1) && freeAt(x, y - 1)) {
            jumpFrom(idx, g, x, y, 0, -1);
            jumpFrom(idx, g, x, y, dx, -1);
        }
    } else {
        jumpFrom(idx, g, x, y, 0, dy);
        if (!freeAt(x + 1, y - dy) && freeAt(x + 1, y)) {
            jumpFrom(idx, g, x, y, 1, 0);
            jumpFrom(idx, g, x, y, 1, dy);
        }
        if (!freeAt(x - 1, y - dy) && freeAt(x - 1, y)) {
            jumpFrom(idx, g, x, y, -1, 0);
            jumpFrom(idx, g, x, y, -1, dy);
        }
    }
}

void JpsSearch::finishFromParents() {
    std::vector<Point> waypoints;
    for (int i = goalIdx_; i != -1; i = parent_[i]) {
        waypoints.push_back({ i % W_, i / W_ });
    }
    std::reverse(waypoints.begin(), waypoints.end());

    std::vector<Point> cells;
    cells.push_back(waypoints[0]);
    for (size_t k = 1; k < waypoints.size(); ++k) {
        Point at = waypoints[k - 1];
        const Point to = waypoints[k];
        const int dx = (to.x > at.x) - (to.x < at.x);
        const int dy = (to.y > at.y) - (to.y < at.y);
        while (at != to) {
            at = { at.x + dx, at.y + dy };
            cells.push_back(at);
        }
    }
    finishWithPath(std::move(cells));
}
