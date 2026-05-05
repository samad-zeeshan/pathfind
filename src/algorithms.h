// Algorithm registry. The UI, tests, and benchmark enumerate algorithms from
// here, so adding one means implementing Pathfinder and adding a line to the
// table in algorithms.cpp.

#pragma once

#include "pathfinder.h"

#include <memory>

int algorithmCount();
const char* algorithmName(int index);
std::unique_ptr<Pathfinder> makeAlgorithm(int index);
