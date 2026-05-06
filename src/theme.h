// The design tokens for the whole UI: palette, spacing, and type scale.
// Every drawing site reads from here so the theme changes in one place.

#pragma once

#include "raylib.h"

namespace theme {

// Surface colors for the window, cards, grid, and insets. The grid floor sits
// above the window backing so the board reads as the content plane.
constexpr Color kWindowBg   = { 15, 17, 22, 255 };    // #0f1116
constexpr Color kGridBg     = { 16, 19, 26, 255 };    // backing card behind the board
constexpr Color kFloor      = { 29, 33, 43, 255 };    // #1d212b
constexpr Color kGridLine   = { 21, 25, 35, 255 };    // hairline between cells
constexpr Color kPanel      = { 22, 25, 34, 255 };    // #161922
constexpr Color kPanelInset = { 16, 19, 26, 255 };    // recessed cards inside the panel
constexpr Color kBorder     = { 38, 43, 54, 255 };    // #262b36

// Search-state fills, validated as a set against the floor for CVD separation
// and contrast (frontier/expanded/path worst adjacent deltaE 12.7). Alphas are
// part of the validated result, change them together with the hue.
constexpr Color kWall     = { 184, 191, 206, 255 };   // #b8bfce
constexpr Color kFrontier = { 77, 163, 255, 178 };    // #4da3ff @ 70%
constexpr Color kExpanded = { 143, 124, 240, 140 };   // #8f7cf0 @ 55%
// The halo is #c98500 at 42% precomposited over the floor and drawn opaque,
// because overlapping translucent segments would stack into blotches at joints.
constexpr Color kPathGlow = { 101, 75, 25, 255 };
constexpr Color kPathCore = { 237, 161, 0, 255 };     // #eda100 line pass

// Start and goal are red/green, so the markers also differ in shape (disc vs
// ring). Color is never the only channel.
constexpr Color kStart = { 29, 185, 84, 255 };        // #1db954
constexpr Color kGoal  = { 227, 73, 72, 255 };        // #e34948

constexpr Color kAccent  = { 77, 163, 255, 255 };     // interactive / running
constexpr Color kGold    = { 237, 161, 0, 255 };      // found
constexpr Color kDanger  = { 227, 73, 72, 255 };      // no path
constexpr Color kWarn    = { 250, 178, 25, 255 };     // paused

// Ink for text. The accent above doubles as an emphasis ink on the active row
// and the wordmark, everything else uses these three.
constexpr Color kText      = { 236, 238, 243, 255 };
constexpr Color kTextDim   = { 154, 161, 178, 255 };
constexpr Color kTextMuted = { 108, 115, 132, 255 };

// Layout lives in config.h with the window math. These are purely cosmetic.
constexpr int kPanelPad = 16;
constexpr int kRadius   = 10;   // card corner radius in pixels

// Type scale.
constexpr int kFontTitle = 22;
constexpr int kFontBody  = 15;
constexpr int kFontSmall = 13;
constexpr int kFontLabel = 12;    // letterspaced section headers

}  // namespace theme
