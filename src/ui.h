// Reusable UI components: text, cards, badges, stat rows, keycaps, legend.
// Immediate-mode, stateless between frames. The composition lives in main.cpp.

#pragma once

#include "raylib.h"

void initUi();       // loads the embedded font
void shutdownUi();

// Text primitives. All UI text goes through these so the font and spacing
// policy stay in one place.
void uiText(const char* text, int x, int y, int size, Color color);
void uiTextRight(const char* text, int rightX, int y, int size, Color color);
int  uiTextWidth(const char* text, int size);

// Letterspaced small-caps style header, e.g. "SEARCH".
void uiSectionLabel(const char* text, int x, int y);

// Rounded surface with a hairline border.
void uiCard(Rectangle rec, Color fill);

// Pill with a status dot. Returns the pill width so callers can stack them.
int uiBadge(const char* text, int x, int y, Color dotColor);

// Label left, value right, inside a row of the given width.
void uiStatRow(const char* label, const char* value, int x, int y, int width);

// One row of the algorithm list. Active rows get the accent treatment.
void uiListRow(const char* key, const char* name, int x, int y, int width, bool active);

// Keyboard hint: a keycap chip followed by a muted description.
// Returns the total advance so hints can flow in a row.
int uiKeyHint(const char* key, const char* desc, int x, int y);

// Color swatch plus label, for the legend. Swatches with alpha are drawn on a
// floor backing so they match what the grid actually shows.
void uiLegendItem(const char* label, Color swatch, bool onFloor, int x, int y);
