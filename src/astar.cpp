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

AStarSearch::AStarSearch(const Grid& grid, int sx, int sy, int gx, int gy)
    : grid_(grid),
      W_(grid.width()),
      H_(grid.height()),
      gx_(gx),
      gy_(gy),
      goalIdx_(gy * grid.width() + gx),
      status_(SearchStatus::Running),
      g_(grid.width() * grid.height(), kInf),
      parent_(grid.width() * grid.height(), -1),
      closed_(grid.width() * grid.height(), 0),
      inOpen_(grid.width() * grid.height(), 0)
{
    if (!grid.inBounds(sx, sy) || !grid.inBounds(gx, gy) ||
        grid.at(sx, sy) == Cell::Wall || grid.at(gx, gy) == Cell::Wall) {
        status_ = SearchStatus::NoPath;
        return;
    }
    const int startIdx = sy * W_ + sx;
    g_[startIdx]      = 0;
    inOpen_[startIdx] = 1;
    open_.push({ startIdx, octile(sx - gx, sy - gy), 0 });
}

SearchStatus AStarSearch::step() {
    if (status_ != SearchStatus::Running) return status_;

    while (!open_.empty()) {
        const OpenNode cur = open_.top();
        open_.pop();
        if (closed_[cur.idx]) continue;
        if (cur.g != g_[cur.idx]) continue;
        closed_[cur.idx] = 1;
        inOpen_[cur.idx] = 0;
        ++nodesExpanded_;

        if (cur.idx == goalIdx_) {
            status_ = SearchStatus::Found;
            return status_;
        }

        const int cx = cur.idx % W_;
        const int cy = cur.idx / W_;
        for (int d = 0; d < 8; ++d) {
            const int nx = cx + DX[d];
            const int ny = cy + DY[d];
            if (!grid_.inBounds(nx, ny)) continue;
            if (grid_.at(nx, ny) == Cell::Wall) continue;
            if (DX[d] != 0 && DY[d] != 0) {
                if (grid_.at(cx + DX[d], cy) == Cell::Wall) continue;
                if (grid_.at(cx, cy + DY[d]) == Cell::Wall) continue;
            }
            const int nIdx = ny * W_ + nx;
            if (closed_[nIdx]) continue;

            const int newG = cur.g + CST[d];
            if (newG < g_[nIdx]) {
                g_[nIdx]      = newG;
                parent_[nIdx] = cur.idx;
                inOpen_[nIdx] = 1;
                open_.push({ nIdx, newG + octile(nx - gx_, ny - gy_), newG });
            }
        }
        return status_;  // one expansion per step()
    }

    status_ = SearchStatus::NoPath;
    return status_;
}

SearchStatus AStarSearch::runToEnd() {
    while (status_ == SearchStatus::Running) step();
    return status_;
}

bool AStarSearch::isOpen(int x, int y) const {
    return inOpen_[y * W_ + x] != 0;
}

bool AStarSearch::isClosed(int x, int y) const {
    return closed_[y * W_ + x] != 0;
}

std::vector<std::pair<int, int>> AStarSearch::path() const {
    std::vector<std::pair<int, int>> result;
    if (status_ != SearchStatus::Found) return result;
    for (int i = goalIdx_; i != -1; i = parent_[i]) {
        result.push_back({ i % W_, i / W_ });
    }
    std::reverse(result.begin(), result.end());
    return result;
}

AStarResult astar(const Grid& grid, int sx, int sy, int gx, int gy) {
    AStarSearch s(grid, sx, sy, gx, gy);
    s.runToEnd();
    AStarResult r;
    r.found         = s.status() == SearchStatus::Found;
    r.nodesExpanded = s.nodesExpanded();
    if (r.found) r.path = s.path();
    return r;
}
