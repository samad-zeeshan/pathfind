// Command-line parsing for the native window. Web ignores this and keeps defaults.

#include "config.h"

#include <cstdlib>
#include <cstring>

namespace {

// Read "--flag N" into out. Advances i past the value. Rejects anything that is
// not a positive integer so a typo cannot silently produce a zero-sized grid.
bool takeInt(int argc, char** argv, int& i, int& out) {
    if (i + 1 >= argc) return false;
    char* end = nullptr;
    const long v = std::strtol(argv[i + 1], &end, 10);
    if (end == argv[i + 1] || *end != '\0' || v <= 0) return false;
    out = (int)v;
    ++i;
    return true;
}

}  // namespace

bool parseArgs(int argc, char** argv, Config& cfg) {
    for (int i = 1; i < argc; ++i) {
        bool ok = true;
        if (std::strcmp(argv[i], "--cols") == 0)           ok = takeInt(argc, argv, i, cfg.cols);
        else if (std::strcmp(argv[i], "--rows") == 0)      ok = takeInt(argc, argv, i, cfg.rows);
        else if (std::strcmp(argv[i], "--cell-size") == 0) ok = takeInt(argc, argv, i, cfg.cellSize);
        else return false;
        if (!ok) return false;
    }
    return true;
}
