#pragma once

#include <cstdint>
#include <queue>
#include <utility>
#include <vector>

class Grid;

enum class SearchStatus { Running, Found, NoPath };

class AStarSearch {
public:
    AStarSearch(const Grid& grid, int sx, int sy, int gx, int gy);

    SearchStatus step();        // expand at most one node
    SearchStatus runToEnd();    // step() until done
    SearchStatus status() const { return status_; }
    int nodesExpanded() const { return nodesExpanded_; }
    int width() const { return W_; }
    int height() const { return H_; }

    bool isOpen(int x, int y) const;
    bool isClosed(int x, int y) const;
    std::vector<std::pair<int, int>> path() const; // empty unless Found

    struct OpenNode { int idx; int f; int g; };
    struct OpenCmp  { bool operator()(const OpenNode& a, const OpenNode& b) const; };

private:
    const Grid& grid_;
    int W_, H_;
    int gx_, gy_;
    int goalIdx_;
    SearchStatus status_;
    int nodesExpanded_ = 0;
    std::vector<int>     g_;
    std::vector<int>     parent_;
    std::vector<uint8_t> closed_;
    std::vector<uint8_t> inOpen_;
    std::priority_queue<OpenNode, std::vector<OpenNode>, OpenCmp> open_;
};

struct AStarResult {
    std::vector<std::pair<int, int>> path;
    int nodesExpanded = 0;
    bool found = false;
};

AStarResult astar(const Grid& grid, int sx, int sy, int gx, int gy);
