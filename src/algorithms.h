// Algorithm registry. The UI, tests, and benchmark enumerate algorithms from
// here, so adding one means implementing Pathfinder and adding a line to the
// table in algorithms.cpp.

#pragma once

#include "pathfinder.h"

#include <memory>

int algorithmCount();
const char* algorithmName(int index);

// Registry position of a named algorithm, -1 if absent. Callers pin a default
// by name so appending to the table never silently retargets it.
int algorithmIndex(const char* name);

std::unique_ptr<Pathfinder> makeAlgorithm(int index);
