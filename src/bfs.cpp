// BFS implementation. Cells are marked at discovery, so each enters the queue
// exactly once and no staleness handling is needed.

#include "bfs.h"

#include "grid.h"

void BfsSearch::seed(int startIdx) {
    g_[startIdx] = 0;
    inOpen_[startIdx] = 1;
    openCount_ = 1;
    queue_.push(startIdx);
}

SearchStatus BfsSearch::step() {
    if (status_ != SearchStatus::Running) return status_;

    if (queue_.empty()) {
        status_ = SearchStatus::NoPath;
        return status_;
    }

    const int cur = queue_.front();
    queue_.pop();
    closed_[cur] = 1;
    inOpen_[cur] = 0;
    --openCount_;
    ++nodesExpanded_;

    if (cur == goalIdx_) {
        finishFromParents();
        return status_;
    }

    Neighbor nbr[8];
    const int count = neighbors(cur % W_, cur / W_, nbr);
    for (int i = 0; i < count; ++i) {
        const int nIdx = index(nbr[i].x, nbr[i].y);
        if (closed_[nIdx] || inOpen_[nIdx]) continue;
        g_[nIdx] = g_[cur] + 1;
        parent_[nIdx] = cur;
        inOpen_[nIdx] = 1;
        ++openCount_;
        queue_.push(nIdx);
    }
    return status_;
}
