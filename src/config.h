// Grid and window sizing. Defaults reproduce the original 40x30 board; the
// native build can override them from the command line.

#pragma once

#include <algorithm>

struct Config {
    int cols = 40;
    int rows = 30;
    int cellSize = 18;
};

// Layout is grid on the left, a fixed-width stats panel on the right. The grid
// origin is the same margin on both axes.
constexpr int kMargin = 20;
constexpr int kPanelWidth = 250;

inline int screenWidth(const Config& c) {
    return kMargin + c.cols * c.cellSize + kMargin + kPanelWidth;
}

inline int screenHeight(const Config& c) {
    const int gridBottom = kMargin + c.rows * c.cellSize;
    // Room under the grid for the two help lines, with a floor so the panel and
    // legend still fit when the grid is tiny.
    return std::max(gridBottom + 60, 420);
}

// Parse --cols, --rows, --cell-size into cfg. Returns false on a missing or
// non-positive value so main can print usage and exit instead of guessing.
bool parseArgs(int argc, char** argv, Config& cfg);
