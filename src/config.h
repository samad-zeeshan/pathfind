// Grid and window sizing. Defaults reproduce the original 40x30 board. The
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
constexpr int kPanelWidth = 260;
constexpr int kHelpBand = 64;         // reserved strip under the grid for key hints
constexpr int kPanelMinBottom = 580;  // panel content height floor, see drawPanel

inline int screenWidth(const Config& c) {
    return kMargin + c.cols * c.cellSize + kMargin + kPanelWidth;
}

inline int screenHeight(const Config& c) {
    const int gridBottom = kMargin + c.rows * c.cellSize;
    // The help band hangs below whichever column is taller, so tiny grids still
    // fit the full panel and huge grids keep their hints.
    return std::max(gridBottom, kPanelMinBottom) + kHelpBand;
}

// Parse --cols, --rows, --cell-size into cfg. Returns false on a missing or
// non-positive value so main can print usage and exit instead of guessing.
bool parseArgs(int argc, char** argv, Config& cfg);
