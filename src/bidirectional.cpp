// Bidirectional A* implementation.
//
// Termination is the subtle part: the first meeting of the frontiers is not
// necessarily on a shortest path. The search keeps the cheapest meeting seen
// so far and only stops once that cost is <= the larger of the two frontier
// minimums, which is safe for consistent front-to-end heuristics like octile.

#include "bidirectional.h"

#include "grid.h"

#include <algorithm>

void BidirAStarSearch::resetDerived() {
    open_[0] = {};
    open_[1] = {};
    gB_.assign(W_ * H_, kSearchInf);
    parentB_.assign(W_ * H_, -1);
    closedB_.assign(W_ * H_, 0);
    inOpenB_.assign(W_ * H_, 0);
    meetIdx_ = -1;
    bestMeetCost_ = kSearchInf;
}

void BidirAStarSearch::seed(int startIdx) {
    if (startIdx == goalIdx_) {
        finishWithPath({ start_ });
        return;
    }
    g_[startIdx] = 0;
    inOpen_[startIdx] = 1;
    open_[0].push({ startIdx, heuristicFor(0, start_.x, start_.y), 0 });
    gB_[goalIdx_] = 0;
    inOpenB_[goalIdx_] = 1;
    open_[1].push({ goalIdx_, heuristicFor(1, goal_.x, goal_.y), 0 });
    openCount_ = 2;
}

int BidirAStarSearch::heuristicFor(int dir, int x, int y) const {
    const Point target = dir == 0 ? goal_ : start_;
    return octileDistance(x - target.x, y - target.y, cost_);
}

bool BidirAStarSearch::isOpen(Point p) const {
    if (!grid_ || !grid_->inBounds(p.x, p.y)) return false;
    const int i = index(p.x, p.y);
    return inOpen_[i] != 0 || inOpenB_[i] != 0;
}

bool BidirAStarSearch::isClosed(Point p) const {
    if (!grid_ || !grid_->inBounds(p.x, p.y)) return false;
    const int i = index(p.x, p.y);
    return closed_[i] != 0 || closedB_[i] != 0;
}

void BidirAStarSearch::purge(int dir) {
    const std::vector<int>& g = dir == 0 ? g_ : gB_;
    const std::vector<uint8_t>& closed = dir == 0 ? closed_ : closedB_;
    Heap& heap = open_[dir];
    while (!heap.empty() &&
           (closed[heap.top().idx] || heap.top().g != g[heap.top().idx])) {
        heap.pop();
    }
}

void BidirAStarSearch::considerMeeting(int idx) {
    if (g_[idx] == kSearchInf || gB_[idx] == kSearchInf) return;
    const int total = g_[idx] + gB_[idx];
    if (total < bestMeetCost_) {
        bestMeetCost_ = total;
        meetIdx_ = idx;
    }
}

SearchStatus BidirAStarSearch::step() {
    if (status_ != SearchStatus::Running) return status_;

    // Purge first so the frontier minimums read below are live values.
    purge(0);
    purge(1);

    if (bestMeetCost_ < kSearchInf) {
        const int f0 = open_[0].empty() ? kSearchInf : open_[0].top().key;
        const int f1 = open_[1].empty() ? kSearchInf : open_[1].top().key;
        if (bestMeetCost_ <= std::max(f0, f1)) {
            finishThroughMeet();
            return status_;
        }
    }

    // An exhausted side with no meeting means the endpoints are in different
    // components, the other side can never reach anything new.
    if (open_[0].empty() || open_[1].empty()) {
        status_ = SearchStatus::NoPath;
        return status_;
    }

    expandDirection(open_[0].top().key <= open_[1].top().key ? 0 : 1);
    return status_;
}

void BidirAStarSearch::expandDirection(int dir) {
    std::vector<int>& g = dir == 0 ? g_ : gB_;
    std::vector<int>& parent = dir == 0 ? parent_ : parentB_;
    std::vector<uint8_t>& closed = dir == 0 ? closed_ : closedB_;
    std::vector<uint8_t>& inOpen = dir == 0 ? inOpen_ : inOpenB_;

    const OpenNode cur = open_[dir].top();
    open_[dir].pop();
    closed[cur.idx] = 1;
    if (inOpen[cur.idx]) { inOpen[cur.idx] = 0; --openCount_; }
    ++nodesExpanded_;
    considerMeeting(cur.idx);

    Neighbor nbr[8];
    const int count = neighbors(cur.idx % W_, cur.idx / W_, nbr);
    for (int i = 0; i < count; ++i) {
        const int nIdx = index(nbr[i].x, nbr[i].y);
        if (closed[nIdx]) continue;
        const int newG = cur.g + nbr[i].cost;
        if (newG >= g[nIdx]) continue;
        g[nIdx] = newG;
        parent[nIdx] = cur.idx;
        if (!inOpen[nIdx]) { inOpen[nIdx] = 1; ++openCount_; }
        open_[dir].push({ nIdx, newG + heuristicFor(dir, nbr[i].x, nbr[i].y), newG });
        considerMeeting(nIdx);
    }
}

void BidirAStarSearch::finishThroughMeet() {
    std::vector<Point> cells;
    for (int i = meetIdx_; i != -1; i = parent_[i]) {
        cells.push_back({ i % W_, i / W_ });
    }
    std::reverse(cells.begin(), cells.end());
    for (int i = parentB_[meetIdx_]; i != -1; i = parentB_[i]) {
        cells.push_back({ i % W_, i / W_ });
    }
    finishWithPath(std::move(cells));
}
