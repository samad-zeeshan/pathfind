// Shared machinery for grid searches: cost model, neighbor rules, state arrays,
// and the best-first heap that A*, Dijkstra, and Greedy specialize.

#pragma once

#include "pathfinder.h"

#include <cstdint>
#include <limits>
#include <queue>
#include <vector>

class Grid;

struct CostModel {
    int straight;
    int diagonal;
};

// Costs are integers scaled by 1000 so optimal algorithms can be compared for
// exact cost equality with no float drift. 1414 approximates sqrt(2).
constexpr CostModel kOctileCosts{ 1000, 1414 };

// Every move costs 1. This is what BFS effectively assumes, exposed as a model
// so the tests can run Dijkstra on the same terms.
constexpr CostModel kUnitCosts{ 1, 1 };

constexpr int kSearchInf = std::numeric_limits<int>::max() / 4;

constexpr int kDX[8] = { 1, -1, 0,  0, 1,  1, -1, -1 };
constexpr int kDY[8] = { 0,  0, 1, -1, 1, -1,  1, -1 };

// Octile distance under the given cost model. Admissible for both models: with
// unit costs it degrades to Chebyshev distance, a lower bound on 8-way hops.
int octileDistance(int dx, int dy, CostModel cost);

struct Neighbor {
    int x, y;
    int cost;
};

// Holds everything the algorithms have in common: the per-cell state arrays,
// endpoint validation, neighbor generation, and path reconstruction. Concrete
// algorithms supply the frontier and the expansion order.
class GridSearchBase : public Pathfinder {
public:
    void init(const Grid& grid, Point start, Point goal) override;
    SearchStatus status() const override { return status_; }

    bool isOpen(Point p) const override;
    bool isClosed(Point p) const override;
    const std::vector<Point>& path() const override { return path_; }
    SearchStats stats() const override;

protected:
    explicit GridSearchBase(CostModel cost) : cost_(cost) {}

    // Called by init() after the shared state is reset and endpoints validated.
    virtual void resetDerived() = 0;
    virtual void seed(int startIdx) = 0;

    int index(int x, int y) const { return y * W_ + x; }

    // Applies the movement rules in one place: walls block, and a diagonal
    // needs both orthogonal cells free (no corner cutting). Returns count.
    int neighbors(int cx, int cy, Neighbor out[8]) const;

    // Path cost is recomputed from the returned cells rather than read from g,
    // so it is honest even for algorithms whose g is not cost-optimal.
    void finishWithPath(std::vector<Point> cells);

    // Virtual because parents are not always adjacent, JPS chains jump points
    // and has to fill in the cells between them.
    virtual void finishFromParents();

    const Grid* grid_ = nullptr;
    int W_ = 0, H_ = 0;
    Point start_{}, goal_{};
    int startIdx_ = -1, goalIdx_ = -1;
    CostModel cost_;
    SearchStatus status_ = SearchStatus::NoPath;
    int nodesExpanded_ = 0;
    int openCount_ = 0;
    int pathCost_ = 0;
    std::vector<int>     g_;
    std::vector<int>     parent_;
    std::vector<uint8_t> closed_;
    std::vector<uint8_t> inOpen_;
    std::vector<Point>   path_;
};

// Best-first search over a binary heap. A*, Dijkstra, and Greedy are this class
// with different priority functions.
class HeapSearch : public GridSearchBase {
public:
    SearchStatus step() override;

protected:
    explicit HeapSearch(CostModel cost) : GridSearchBase(cost) {}

    // Orders the frontier. g is the cost from the start, h the heuristic.
    virtual int priority(int g, int h) const = 0;

    // Decides whether a neighbor discovered at cost newG enters the frontier.
    // Default is the Dijkstra/A* rule, requeue on any strict improvement.
    virtual bool tryRelax(int nIdx, int newG);

    // Called once per expansion with the node just closed. The default visits
    // the adjacent cells, JPS overrides it to jump to distant successors.
    virtual void expandNode(int idx, int g);

    // Relax toIdx at cost newG and push it, shared by every successor source.
    void enqueue(int fromIdx, int toIdx, int newG);

    int heuristic(int x, int y) const {
        return octileDistance(x - goal_.x, y - goal_.y, cost_);
    }

private:
    struct OpenNode { int idx; int key; int g; };
    struct OpenCmp {
        // Min-heap on key, then larger g first (prefer deeper nodes among
        // ties), then smaller index so runs are reproducible.
        bool operator()(const OpenNode& a, const OpenNode& b) const {
            if (a.key != b.key) return a.key > b.key;
            if (a.g != b.g) return a.g < b.g;
            return a.idx > b.idx;
        }
    };
    std::priority_queue<OpenNode, std::vector<OpenNode>, OpenCmp> open_;

    void resetDerived() override { open_ = {}; }
    void seed(int startIdx) override;
};
