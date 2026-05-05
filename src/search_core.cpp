// Implementation of the shared search machinery.

#include "search_core.h"

#include "grid.h"

#include <algorithm>
#include <cstdlib>

int octileDistance(int dx, int dy, CostModel cost) {
    dx = std::abs(dx);
    dy = std::abs(dy);
    const int mn = std::min(dx, dy);
    const int mx = std::max(dx, dy);
    return cost.straight * (mx - mn) + cost.diagonal * mn;
}

void GridSearchBase::init(const Grid& grid, Point start, Point goal) {
    grid_ = &grid;
    W_ = grid.width();
    H_ = grid.height();
    start_ = start;
    goal_ = goal;
    startIdx_ = index(start.x, start.y);
    goalIdx_ = index(goal.x, goal.y);
    status_ = SearchStatus::Running;
    nodesExpanded_ = 0;
    openCount_ = 0;
    pathCost_ = 0;
    g_.assign(W_ * H_, kSearchInf);
    parent_.assign(W_ * H_, -1);
    closed_.assign(W_ * H_, 0);
    inOpen_.assign(W_ * H_, 0);
    path_.clear();
    resetDerived();

    if (!grid.inBounds(start.x, start.y) || !grid.inBounds(goal.x, goal.y) ||
        grid.at(start.x, start.y) == Cell::Wall || grid.at(goal.x, goal.y) == Cell::Wall) {
        status_ = SearchStatus::NoPath;
        return;
    }
    seed(startIdx_);
}

int GridSearchBase::neighbors(int cx, int cy, Neighbor out[8]) const {
    int n = 0;
    for (int d = 0; d < 8; ++d) {
        const int nx = cx + kDX[d];
        const int ny = cy + kDY[d];
        if (!grid_->inBounds(nx, ny)) continue;
        if (grid_->at(nx, ny) == Cell::Wall) continue;
        const bool diagonal = kDX[d] != 0 && kDY[d] != 0;
        if (diagonal) {
            if (grid_->at(cx + kDX[d], cy) == Cell::Wall) continue;
            if (grid_->at(cx, cy + kDY[d]) == Cell::Wall) continue;
        }
        out[n++] = { nx, ny, diagonal ? cost_.diagonal : cost_.straight };
    }
    return n;
}

bool GridSearchBase::isOpen(Point p) const {
    if (!grid_ || !grid_->inBounds(p.x, p.y)) return false;
    return inOpen_[index(p.x, p.y)] != 0;
}

bool GridSearchBase::isClosed(Point p) const {
    if (!grid_ || !grid_->inBounds(p.x, p.y)) return false;
    return closed_[index(p.x, p.y)] != 0;
}

SearchStats GridSearchBase::stats() const {
    SearchStats s;
    s.nodesExpanded = nodesExpanded_;
    s.frontierSize  = openCount_;
    s.pathLength    = (int)path_.size();
    s.pathCost      = pathCost_;
    return s;
}

void GridSearchBase::finishWithPath(std::vector<Point> cells) {
    path_ = std::move(cells);
    pathCost_ = 0;
    for (size_t k = 1; k < path_.size(); ++k) {
        const bool diagonal = path_[k].x != path_[k - 1].x && path_[k].y != path_[k - 1].y;
        pathCost_ += diagonal ? cost_.diagonal : cost_.straight;
    }
    status_ = SearchStatus::Found;
}

void GridSearchBase::finishFromParents() {
    std::vector<Point> cells;
    for (int i = goalIdx_; i != -1; i = parent_[i]) {
        cells.push_back({ i % W_, i / W_ });
    }
    std::reverse(cells.begin(), cells.end());
    finishWithPath(std::move(cells));
}

bool HeapSearch::tryRelax(int nIdx, int newG) {
    if (newG >= g_[nIdx]) return false;
    g_[nIdx] = newG;
    return true;
}

void HeapSearch::seed(int startIdx) {
    g_[startIdx] = 0;
    inOpen_[startIdx] = 1;
    openCount_ = 1;
    open_.push({ startIdx, priority(0, heuristic(start_.x, start_.y)), 0 });
}

SearchStatus HeapSearch::step() {
    if (status_ != SearchStatus::Running) return status_;

    while (!open_.empty()) {
        const OpenNode cur = open_.top();
        open_.pop();
        // Lazy deletion. A cell can sit in the heap several times, only the
        // entry matching its current g is live.
        if (closed_[cur.idx]) continue;
        if (cur.g != g_[cur.idx]) continue;
        closed_[cur.idx] = 1;
        if (inOpen_[cur.idx]) { inOpen_[cur.idx] = 0; --openCount_; }
        ++nodesExpanded_;

        if (cur.idx == goalIdx_) {
            finishFromParents();
            return status_;
        }

        Neighbor nbr[8];
        const int count = neighbors(cur.idx % W_, cur.idx / W_, nbr);
        for (int i = 0; i < count; ++i) {
            const int nIdx = index(nbr[i].x, nbr[i].y);
            if (closed_[nIdx]) continue;
            if (!tryRelax(nIdx, cur.g + nbr[i].cost)) continue;
            parent_[nIdx] = cur.idx;
            if (!inOpen_[nIdx]) { inOpen_[nIdx] = 1; ++openCount_; }
            open_.push({ nIdx, priority(g_[nIdx], heuristic(nbr[i].x, nbr[i].y)), g_[nIdx] });
        }
        return status_;  // one expansion per step()
    }

    status_ = SearchStatus::NoPath;
    return status_;
}
