// A* implementation. Integer costs, 1000 straight and 1414 diagonal, so optimal
// algorithms can be compared for exact cost equality with no float drift.

#include "astar.h"

#include "grid.h"

#include <algorithm>
#include <cstdlib>
#include <limits>

namespace {

constexpr int kStraight = 1000;
constexpr int kDiagonal = 1414;
constexpr int kInf      = std::numeric_limits<int>::max() / 4;

int octile(int dx, int dy) {
    dx = std::abs(dx);
    dy = std::abs(dy);
    const int mn = std::min(dx, dy);
    const int mx = std::max(dx, dy);
    return kStraight * (mx - mn) + kDiagonal * mn;
}

constexpr int DX[8]  = { 1, -1, 0,  0, 1,  1, -1, -1 };
constexpr int DY[8]  = { 0,  0, 1, -1, 1, -1,  1, -1 };
constexpr int CST[8] = { kStraight, kStraight, kStraight, kStraight,
                         kDiagonal, kDiagonal, kDiagonal, kDiagonal };

}  // namespace

bool AStarSearch::OpenCmp::operator()(const OpenNode& a, const OpenNode& b) const {
    if (a.f != b.f) return a.f > b.f;  // min-heap on f
    return a.g < b.g;                   // tie-break: larger g first
}

void AStarSearch::init(const Grid& grid, Point start, Point goal) {
    grid_ = &grid;
    W_ = grid.width();
    H_ = grid.height();
    goal_ = goal;
    goalIdx_ = goal.y * W_ + goal.x;
    status_ = SearchStatus::Running;
    nodesExpanded_ = 0;
    openCount_ = 0;
    g_.assign(W_ * H_, kInf);
    parent_.assign(W_ * H_, -1);
    closed_.assign(W_ * H_, 0);
    inOpen_.assign(W_ * H_, 0);
    open_ = {};
    path_.clear();

    if (!grid.inBounds(start.x, start.y) || !grid.inBounds(goal.x, goal.y) ||
        grid.at(start.x, start.y) == Cell::Wall || grid.at(goal.x, goal.y) == Cell::Wall) {
        status_ = SearchStatus::NoPath;
        return;
    }
    const int startIdx = start.y * W_ + start.x;
    g_[startIdx]      = 0;
    inOpen_[startIdx] = 1;
    openCount_        = 1;
    open_.push({ startIdx, octile(start.x - goal.x, start.y - goal.y), 0 });
}

SearchStatus AStarSearch::step() {
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
            status_ = SearchStatus::Found;
            reconstructPath();
            return status_;
        }

        const int cx = cur.idx % W_;
        const int cy = cur.idx / W_;
        for (int d = 0; d < 8; ++d) {
            const int nx = cx + DX[d];
            const int ny = cy + DY[d];
            if (!grid_->inBounds(nx, ny)) continue;
            if (grid_->at(nx, ny) == Cell::Wall) continue;
            // No corner cutting, a diagonal needs both orthogonal cells free.
            if (DX[d] != 0 && DY[d] != 0) {
                if (grid_->at(cx + DX[d], cy) == Cell::Wall) continue;
                if (grid_->at(cx, cy + DY[d]) == Cell::Wall) continue;
            }
            const int nIdx = ny * W_ + nx;
            if (closed_[nIdx]) continue;

            const int newG = cur.g + CST[d];
            if (newG < g_[nIdx]) {
                g_[nIdx]      = newG;
                parent_[nIdx] = cur.idx;
                if (!inOpen_[nIdx]) { inOpen_[nIdx] = 1; ++openCount_; }
                open_.push({ nIdx, newG + octile(nx - goal_.x, ny - goal_.y), newG });
            }
        }
        return status_;  // one expansion per step()
    }

    status_ = SearchStatus::NoPath;
    return status_;
}

bool AStarSearch::isOpen(Point p) const {
    if (!grid_ || p.x < 0 || p.y < 0 || p.x >= W_ || p.y >= H_) return false;
    return inOpen_[p.y * W_ + p.x] != 0;
}

bool AStarSearch::isClosed(Point p) const {
    if (!grid_ || p.x < 0 || p.y < 0 || p.x >= W_ || p.y >= H_) return false;
    return closed_[p.y * W_ + p.x] != 0;
}

SearchStats AStarSearch::stats() const {
    SearchStats s;
    s.nodesExpanded = nodesExpanded_;
    s.frontierSize  = openCount_;
    s.pathLength    = (int)path_.size();
    s.pathCost      = status_ == SearchStatus::Found ? g_[goalIdx_] : 0;
    return s;
}

void AStarSearch::reconstructPath() {
    path_.clear();
    for (int i = goalIdx_; i != -1; i = parent_[i]) {
        path_.push_back({ i % W_, i / W_ });
    }
    std::reverse(path_.begin(), path_.end());
}

AStarResult astar(const Grid& grid, int sx, int sy, int gx, int gy) {
    AStarSearch s;
    s.init(grid, { sx, sy }, { gx, gy });
    s.runToEnd();
    AStarResult r;
    r.found         = s.status() == SearchStatus::Found;
    r.nodesExpanded = s.stats().nodesExpanded;
    if (r.found) r.path = s.path();
    return r;
}
