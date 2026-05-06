// UI component implementations. Glyphs come from the embedded JetBrains Mono,
// rasterized once per pixel size so native, web, and the exe render identically.

#include "ui.h"

#include "theme.h"
#include "font_data.h"

namespace {

// One glyph atlas per pixel size we actually draw at. Rasterizing each glyph at
// the exact size it renders keeps text crisp: every glyph draws 1:1 with no
// minification, which is the softness the old single 44 px atlas picked up when
// it scaled down to 12-15 px label text. The type scale only touches a handful
// of sizes, so the cache stays tiny.
struct SizedFont {
    int  size;
    Font font;
    bool owned;   // false when the embedded load failed and we fell back
};

constexpr int kMaxFonts = 8;
SizedFont g_fonts[kMaxFonts];
int       g_fontCount = 0;

// Mono glyphs are on a fixed advance, spacing 0 keeps columns aligned.
constexpr float kSpacing = 0.0f;

float roundness(Rectangle rec, int radiusPx) {
    const float side = rec.width < rec.height ? rec.width : rec.height;
    if (side <= 0) return 0.0f;
    float r = (float)radiusPx / (side * 0.5f);
    return r > 1.0f ? 1.0f : r;
}

const Font& fontForSize(int size) {
    for (int i = 0; i < g_fontCount; ++i)
        if (g_fonts[i].size == size) return g_fonts[i].font;
    if (g_fontCount >= kMaxFonts) return g_fonts[0].font;  // fixed scale never fills this

    Font f = LoadFontFromMemory(".ttf", kUiFontData, (int)kUiFontDataSize,
                                size, nullptr, 0);
    const bool owned = f.texture.id != 0;
    if (owned) {
        // Every glyph is drawn 1:1, so bilinear only smooths its own coverage and
        // never blends mip levels. No mipmaps: nothing here is ever minified.
        SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        f = GetFontDefault();
    }
    g_fonts[g_fontCount] = { size, f, owned };
    return g_fonts[g_fontCount++].font;
}

}  // namespace

void initUi() {
    // Warm the sizes the type scale uses so the first frame takes no load hitch.
    // Any other size is rasterized on demand the first time it is drawn.
    fontForSize(theme::kFontTitle);
    fontForSize(theme::kFontBody);
    fontForSize(theme::kFontSmall);
    fontForSize(theme::kFontLabel);
}

void shutdownUi() {
    for (int i = 0; i < g_fontCount; ++i)
        if (g_fonts[i].owned) UnloadFont(g_fonts[i].font);
    g_fontCount = 0;
}

void uiText(const char* text, int x, int y, int size, Color color) {
    DrawTextEx(fontForSize(size), text, Vector2{ (float)x, (float)y },
               (float)size, kSpacing, color);
}

int uiTextWidth(const char* text, int size) {
    return (int)MeasureTextEx(fontForSize(size), text, (float)size, kSpacing).x;
}

void uiTextRight(const char* text, int rightX, int y, int size, Color color) {
    uiText(text, rightX - uiTextWidth(text, size), y, size, color);
}

void uiSectionLabel(const char* text, int x, int y) {
    DrawTextEx(fontForSize(theme::kFontLabel), text, Vector2{ (float)x, (float)y },
               (float)theme::kFontLabel, 2.0f, theme::kTextMuted);
}

void uiCard(Rectangle rec, Color fill) {
    DrawRectangleRounded(rec, roundness(rec, theme::kRadius), 8, fill);
    DrawRectangleRoundedLinesEx(rec, roundness(rec, theme::kRadius), 8, 1.0f,
                                theme::kBorder);
}

int uiBadge(const char* text, int x, int y, Color dotColor) {
    const int h = 24;
    const int textW = uiTextWidth(text, theme::kFontSmall);
    const int w = 10 + 6 + 8 + textW + 12;
    const Rectangle rec{ (float)x, (float)y, (float)w, (float)h };
    DrawRectangleRounded(rec, 1.0f, 8, theme::kPanelInset);
    DrawRectangleRoundedLinesEx(rec, 1.0f, 8, 1.0f, theme::kBorder);
    DrawCircleV(Vector2{ (float)(x + 13), (float)(y + h / 2) }, 3.5f, dotColor);
    uiText(text, x + 24, y + (h - theme::kFontSmall) / 2, theme::kFontSmall, theme::kText);
    return w;
}

void uiStatRow(const char* label, const char* value, int x, int y, int width) {
    uiText(label, x, y, theme::kFontSmall, theme::kTextDim);
    uiTextRight(value, x + width, y - 1, theme::kFontBody, theme::kText);
}

void uiListRow(const char* key, const char* name, int x, int y, int width, bool active) {
    const int h = 26;
    if (active) {
        const Rectangle rec{ (float)x, (float)y, (float)width, (float)h };
        DrawRectangleRounded(rec, roundness(rec, 6), 8, theme::kPanelInset);
        DrawRectangle(x, y + 4, 3, h - 8, theme::kAccent);
    }
    const Color keyInk  = active ? theme::kAccent : theme::kTextMuted;
    const Color nameInk = active ? theme::kText   : theme::kTextDim;
    uiText(key, x + 12, y + (h - theme::kFontSmall) / 2, theme::kFontSmall, keyInk);
    uiText(name, x + 32, y + (h - theme::kFontBody) / 2, theme::kFontBody, nameInk);
}

int uiKeyHint(const char* key, const char* desc, int x, int y) {
    const int h = 20;
    const int keyW = uiTextWidth(key, theme::kFontLabel) + 12;
    const Rectangle rec{ (float)x, (float)y, (float)keyW, (float)h };
    DrawRectangleRounded(rec, roundness(rec, 5), 8, theme::kPanel);
    DrawRectangleRoundedLinesEx(rec, roundness(rec, 5), 8, 1.0f, theme::kBorder);
    uiText(key, x + 6, y + (h - theme::kFontLabel) / 2, theme::kFontLabel, theme::kTextDim);

    const int descX = x + keyW + 6;
    uiText(desc, descX, y + (h - theme::kFontSmall) / 2, theme::kFontSmall, theme::kTextMuted);
    return keyW + 6 + uiTextWidth(desc, theme::kFontSmall) + 18;
}

void uiLegendItem(const char* label, Color swatch, bool onFloor, int x, int y) {
    const Rectangle rec{ (float)x, (float)y, 12, 12 };
    if (onFloor) DrawRectangleRounded(rec, 0.35f, 6, theme::kFloor);
    DrawRectangleRounded(rec, 0.35f, 6, swatch);
    uiText(label, x + 19, y - 1, theme::kFontSmall, theme::kTextDim);
}
