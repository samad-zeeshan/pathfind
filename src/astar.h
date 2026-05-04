// A* search over the grid with an octile-distance heuristic.

#pragma once

#include "pathfinder.h"

#include <cstdint>
#include <queue>
#include <vector>

class Grid;

class AStarSearch : public Pathfinder {
public:
    void init(const Grid& grid, Point start, Point goal) override;
    SearchStatus step() override;
    SearchStatus status() const override { return status_; }

    bool isOpen(Point p) const override;
    bool isClosed(Point p) const override;
    const std::vector<Point>& path() const override { return path_; }

    SearchStats stats() const override;
    const char* name() const override { return "A*"; }

    struct OpenNode { int idx; int f; int g; };
    struct OpenCmp  { bool operator()(const OpenNode& a, const OpenNode& b) const; };

private:
    const Grid* grid_ = nullptr;
    int W_ = 0, H_ = 0;
    Point goal_{};
    int goalIdx_ = -1;
    SearchStatus status_ = SearchStatus::NoPath;
    int nodesExpanded_ = 0;
    int openCount_ = 0;
    std::vector<int>     g_;
    std::vector<int>     parent_;
    std::vector<uint8_t> closed_;
    std::vector<uint8_t> inOpen_;
    std::priority_queue<OpenNode, std::vector<OpenNode>, OpenCmp> open_;
    std::vector<Point>   path_;

    void reconstructPath();
};

struct AStarResult {
    std::vector<Point> path;
    int nodesExpanded = 0;
    bool found = false;
};

AStarResult astar(const Grid& grid, int sx, int sy, int gx, int gy);
