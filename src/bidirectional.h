// Bidirectional A*: one search from each endpoint, meeting in the middle. The
// explored area is visibly smaller because each frontier only grows to roughly
// half the path length.

#pragma once

#include "search_core.h"

#include <queue>

class BidirAStarSearch : public GridSearchBase {
public:
    BidirAStarSearch() : GridSearchBase(kOctileCosts) {}
    SearchStatus step() override;
    const char* name() const override { return "Bi-A*"; }

    // Overlay shows the union of both searches.
    bool isOpen(Point p) const override;
    bool isClosed(Point p) const override;

private:
    struct OpenNode { int idx; int key; int g; };
    struct OpenCmp {
        bool operator()(const OpenNode& a, const OpenNode& b) const {
            if (a.key != b.key) return a.key > b.key;
            if (a.g != b.g) return a.g < b.g;
            return a.idx > b.idx;
        }
    };
    using Heap = std::priority_queue<OpenNode, std::vector<OpenNode>, OpenCmp>;

    // Direction 0 (forward, from the start) lives in the base arrays.
    // Direction 1 (backward, from the goal) lives in the B-suffixed ones.
    Heap open_[2];
    std::vector<int>     gB_;
    std::vector<int>     parentB_;
    std::vector<uint8_t> closedB_;
    std::vector<uint8_t> inOpenB_;

    int meetIdx_ = -1;
    int bestMeetCost_ = kSearchInf;

    void resetDerived() override;
    void seed(int startIdx) override;

    int heuristicFor(int dir, int x, int y) const;
    void purge(int dir);
    void expandDirection(int dir);
    void considerMeeting(int idx);
    void finishThroughMeet();
};
