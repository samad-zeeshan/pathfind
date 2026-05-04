// The one interface every search algorithm implements. The renderer, the playback
// controller, the tests, and the benchmark all program against this and never
// touch a concrete algorithm.

#pragma once

#include <vector>

class Grid;

struct Point {
    int x = 0;
    int y = 0;
};

inline bool operator==(Point a, Point b) { return a.x == b.x && a.y == b.y; }
inline bool operator!=(Point a, Point b) { return !(a == b); }

enum class SearchStatus { Running, Found, NoPath };

struct SearchStats {
    int nodesExpanded = 0;
    int frontierSize  = 0;   // current open-set size
    int pathLength    = 0;   // cells in the final path, 0 until Found
    int pathCost      = 0;   // integer-scaled cost, 0 until Found
};

class Pathfinder {
public:
    virtual ~Pathfinder() = default;

    // Fresh search on this grid and endpoints. Clears all internal state.
    // The grid must outlive the search, algorithms keep a reference to it.
    virtual void init(const Grid& grid, Point start, Point goal) = 0;

    // Expand exactly one node. The caller owns the pace, which is what makes
    // stepping and animation work.
    virtual SearchStatus step() = 0;
    virtual SearchStatus status() const = 0;

    // Introspection for the renderer overlay and the tests.
    virtual bool isOpen(Point p) const = 0;    // in the frontier
    virtual bool isClosed(Point p) const = 0;  // already expanded
    virtual const std::vector<Point>& path() const = 0;  // valid once Found

    virtual SearchStats stats() const = 0;
    virtual const char* name() const = 0;

    SearchStatus runToEnd() {
        while (step() == SearchStatus::Running) {}
        return status();
    }
};
